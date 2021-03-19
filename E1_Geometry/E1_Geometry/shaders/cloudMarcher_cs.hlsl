
SamplerState sampler0 : register(s0);
RWTexture2D<float4> Result : register(u0);
Texture2D<float4> Source : register(t0);
Texture2D<float4> depthMap : register(t1);
Texture3D<float4> shapeNoiseTex : register(t2);
Texture3D<float4> detailNoiseTex : register(t3);

cbuffer CameraBuffer : register(b0)
{
    matrix invViewMatrix, invProjectionMatrix;
    float3 cameraPos;
    float padding;
};

cbuffer ContainerInfoBuffer : register(b1)
{
    float4 containerBoundsMin;
    float4 containerBoundsMax;
}

cbuffer CloudSettingsBuffer : register(b2)
{
    float4 shapeNoiseTexTransform; // Offset = (x,y,z), Scale = w
    float4 shapeNoiseWeights;
    float4 detailNoiseTexTransform; // Offset = (x,y,z), Scale = w
    float4 detailNoiseWeights;
    float4 densitySettings; // Density Threshold = x, Density Multiplier = y, Density Steps = z
}

cbuffer LightBuffer : register(b3)
{
    float4 lightDir;
    float4 lightPos;
    float4 lightDiffuse;
    float4 lightAbsorptionData; // Absorption to sun = x, Absorption through cloud = y, Darkness Threshold = z, Marching steps = w
}

struct Ray
{
    float3 origin;
    float3 direction;
};

float3 GetViewVector(float2 uv)
{
    float3 viewVector = mul(invProjectionMatrix, float4(uv, 0, 1)).xyz;
    viewVector = mul(invViewMatrix, float4(viewVector, 0)).xyz;
    return viewVector;
}

float GetLinearDepth(float2 uv)
{
    float nonLinearDepth = depthMap.SampleLevel(sampler0, uv, 0);
    float near = 0.1f;
    float far = 1000.f;
    float linearDepth = (2.0f * near) / (far + near - nonLinearDepth * (far - near));
    return linearDepth;
}

float LinearEyeDepth(float z)
{
    z = 1 - z;
    float near = 0.1f;
    float far = 1000.f;
    float4 _ZBufferParams;
    _ZBufferParams.x = -1 + far / near;
    _ZBufferParams.y = 1;
    _ZBufferParams.z = _ZBufferParams.x / far;
    _ZBufferParams.w = 1 / far;

    return 1.0 / (_ZBufferParams.z * z + _ZBufferParams.w);
}

Ray CreateRay(float3 origin, float3 direction) 
{
    Ray ray;
    ray.origin = origin;
    ray.direction = direction;
    return ray;
}

Ray CreateCameraRay(float2 uv) 
{
    float3 origin = mul(invViewMatrix, float4(0, 0, 0, 1)).xyz;
    float3 direction = GetViewVector(uv);
    direction = normalize(direction);
    return CreateRay(origin, direction);
}

// Slabs implementation
float2 RayBoxDst(float3 boundsMin, float3 boundsMax, float3 rayOrigin, float3 rayDir) 
{
    // Inverse ray direction to stop division by zero
    float3 invRaydir = 1 / rayDir;

    // Get the points at which the ray insects each axis
    float3 t0 = (boundsMin - rayOrigin) * invRaydir;
    float3 t1 = (boundsMax - rayOrigin) * invRaydir;

    // Get the min and max of the intersecting points to get the near and far intersection points if there are intersections
    float3 tmin = min(t0, t1);
    float3 tmax = max(t0, t1);
    float dstA = max(max(tmin.x, tmin.y), tmin.z);
    float dstB = min(tmax.x, min(tmax.y, tmax.z));

    // Get the max distance of 0 and the distance to the box as dstA could be negative if the ray origin is within the box
    float dstToBox = max(0, dstA);

    // Calculate the distance inside the box by taking away the distance to the box and making sure it's not negative
    float dstInsideBox = max(0, dstB - dstToBox);

    // Return the 2 distances
    return float2(dstToBox, dstInsideBox);
}

float remap(float v, float minOld, float maxOld, float minNew, float maxNew)
{
    return minNew + (v - minOld) * (maxNew - minNew) / (maxOld - minOld);
}

float SampleDensity(float3 pos)
{
    float densityThreshold = densitySettings.x;
    float densityMultiplier = densitySettings.y;

    // Calculate the uvw point of the texture to sample using the current position in space, applying the scale and adding on the offset
    float3 uvw = pos;
    float3 shapeSamplePos = uvw / shapeNoiseTexTransform.w + shapeNoiseTexTransform.xyz;

    // Calculate falloff at along x/z edges of the cloud container
    const float containerEdgeFadeDst = 50;
    float dstFromEdgeX = min(containerEdgeFadeDst, min(pos.x - containerBoundsMin.x, containerBoundsMax.x - pos.x));
    float dstFromEdgeZ = min(containerEdgeFadeDst, min(pos.z - containerBoundsMin.z, containerBoundsMax.z - pos.z));
    float edgeWeight = min(dstFromEdgeZ, dstFromEdgeX) / containerEdgeFadeDst;

    // Calculate height gradient from weather map
    float3 size = containerBoundsMax - containerBoundsMin;
    float gMin = .2;
    float gMax = .7;
    float heightPercent = (pos.y - containerBoundsMin.y) / size.y;
    float heightGradient = saturate(remap(heightPercent, 0.0, gMin, 0, 1)) * saturate(remap(heightPercent, 1, gMax, 0, 1));
    heightGradient *= edgeWeight;

    // Sample the shape noise texture at the current point
    float4 shapeNoise = shapeNoiseTex.SampleLevel(sampler0, shapeSamplePos, 0);
    float4 normalisedWeights = normalize(shapeNoiseWeights);
    float shapeFBM = dot(shapeNoise, normalisedWeights) * heightGradient;

    // Calculate the density of the shape noise
    float densityOffset = -densityThreshold;
    float shapeDensity = shapeFBM - densityThreshold;

    // If the shape density is not greater than 0 there is no need to calculate the details
    if (shapeDensity > 0)
    {
        // Sample detail noise
        float3 detailSamplePos = uvw / detailNoiseTexTransform.w + detailNoiseTexTransform.xyz;
        float4 detailNoise = detailNoiseTex.SampleLevel(sampler0, detailSamplePos, 0);
        float3 normalizedDetailWeights = normalize(detailNoiseWeights.xyz);
        float detailFBM = dot(detailNoise, normalizedDetailWeights);

        float oneMinusShape = 1 - shapeDensity;
        float detailErodeWeight = oneMinusShape * oneMinusShape * oneMinusShape;
        float cloudDensity = shapeDensity - (1 - detailFBM) * detailErodeWeight;

        return cloudDensity * densityMultiplier;
    }
    return 0;
}

float LightMarch(float3 pos)
{
    // Calculate the distance from the current point to edge of the container in the direction of the light
    float3 dirToLight = -lightDir.xyz;
    float dstInsideBox = RayBoxDst(containerBoundsMin.xyz, containerBoundsMax.xyz, pos, dirToLight).y;

    // March to the light
    int lightSteps = lightAbsorptionData.w;
    float stepSize = dstInsideBox / lightSteps;
    float totalDensity = 0;
    for (int i = 0; i < lightSteps; i++)
    {
        // Sample the density at the current step
        pos += dirToLight * stepSize;
        totalDensity += max(0, SampleDensity(pos) * stepSize);
    }

    // Use beers law to calculate the transmittance of the light as it passes through the container
    float lightAbsTowardSun = lightAbsorptionData.x;
    float darknessThreshold = lightAbsorptionData.z;
    float transmittance = exp(-totalDensity * lightAbsTowardSun);
    return darknessThreshold + transmittance * (1 - darknessThreshold);
}

[numthreads(8, 8, 1)]
void main( int3 id : SV_DispatchThreadID )
{
    // Get the UVs of the screen
    float2 resolution = float2(0, 0);
    Result.GetDimensions(resolution.x, resolution.y);
    float2 uv = (float2(id.x, resolution.y - id.y) / resolution * 2 - 1);

    // Set the colour to the source render texture initially
    float4 col = Source[id.xy];
    Result[id.xy] = col;

    // Calculate the ray origin and direction
    Ray cameraRay = CreateCameraRay(uv);
    float3 ro = cameraRay.origin;
    float3 rd = cameraRay.direction;

    // Calculate the depth
    float3 viewVector = GetViewVector(uv);
    //float depth = depthMap.SampleLevel(sampler0, id.xy / resolution, 0) /** length(viewVector)*/;
    float depth = depthMap[id.xy] /** length(viewVector)*/;
    float linearDepth = LinearEyeDepth(depth) * length(viewVector);

    // Get the ray distance information from the box
    float2 rayBoxInfo = RayBoxDst(containerBoundsMin.xyz, containerBoundsMax.xyz, ro, rd);
    float dstToBox = rayBoxInfo.x;
    float dstInsideBox = rayBoxInfo.y;

    // Ray marching
    int numSteps = densitySettings.z;
    float dstTravelled = 0.0f;
    float transmittance = 1;
    float3 lightEnergy = 0;
    float lightAbsThroughCloud = lightAbsorptionData.y;
    float stepSize = dstInsideBox / numSteps;   // Only march within the container
    float3 entryPoint = ro + rd * dstToBox;     // Point of intersection with the cloud container
    for (int i = 0; i < numSteps; i++)
    {
        // March ray inside the box
        float3 rayPos = entryPoint + rd * dstTravelled;
        float density = SampleDensity(rayPos);

        // Only update the values if the density sampled is greater than zero
        if (density > 0)
        {
            float lightTransmittance = LightMarch(rayPos);
            lightEnergy += density * stepSize * transmittance * lightTransmittance;
            transmittance *= exp(-density * stepSize * lightAbsThroughCloud);

            // If the transmittance is very low, smapling won't effect much, so break
            if (transmittance < 0.01)
                break;
        }

        dstTravelled += stepSize;
    }

    //// Only shade black if inside the box
    //bool rayHitBox = dstInsideBox > 0 && dstToBox < linearDepth;
    //if (rayHitBox)
    //{
    //    col = 0;
    //    //col = shapeNoiseTex.SampleLevel(sampler0, pNear / 10, 0);
    //}

    // Calculate the final colour of the clouds
    float4 cloudCol = lightDiffuse * float4(lightEnergy, 0);
    col = col * transmittance + cloudCol;
    Result[id.xy] = col;
}