#define NUM_THREADS 4

SamplerState sampler0 : register(s0);
RWTexture2D<float4> Result : register(u0);
Texture2D<float4> Source : register(t0);
Texture2D<float4> depthMap : register(t1);
Texture3D<float4> shapeNoiseTex : register(t2);
Texture3D<float4> detailNoiseTex : register(t3);
Texture2D<float4> weatherMapTex : register(t4);
Texture2D<float4> blueNoiseTex : register(t5);
Texture2D<float4> previousFrameTex : register(t6);

cbuffer CameraBuffer : register(b0)
{
    matrix invViewMatrix, invProjectionMatrix, oldViewProjMatrix;
    float3 cameraPos;
    float padding;
};

cbuffer ContainerInfoBuffer : register(b1)
{
    float4 containerBoundsMin;
    float4 containerBoundsMax;
    float edgeFadePercentage;
    float3 padding2;
}

cbuffer CloudSettingsBuffer : register(b2)
{
    float4 shapeNoiseTexTransform; // Offset = (x,y,z), Scale = w
    float4 shapeNoiseWeights;
    float4 detailNoiseTexTransform; // Offset = (x,y,z), Scale = w
    float4 detailNoiseWeights;
    float4 densitySettings; // Global coverage = x, Density Multiplier = y, Density Steps = z, Step Size = w
    float4 optimisationSettings; // Blue noise strength = x, reprojectionFrame = y
}

cbuffer LightBuffer : register(b3)
{
    float4 lightDir;
    float4 lightPos;
    float4 lightDiffuse;
    float4 lightAbsorptionData; // Absorption to sun = x, Absorption through cloud = y, Cloud Brightness = z, Marching steps = w
}

cbuffer WeatherMapBuffer : register(b4)
{
    float4 coverageTexTransform; // Offset = (x,y,z), Scale = w
    float4 weatherMapIntensities; // Channel intensities
};

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
    float globalCoverage = densitySettings.x;
    float densityMultiplier = densitySettings.y;

    // Calculate the uvw point of the texture to sample using the current position in space, applying the scale and adding on the offset
    float3 uvw = pos;
    float3 shapeSamplePos = uvw / shapeNoiseTexTransform.w + shapeNoiseTexTransform.xyz;

    // Calculate falloff at along x/z edges of the cloud container
    const float containerEdgeFadeDst = (containerBoundsMax - containerBoundsMin).x * edgeFadePercentage;
    float dstToEdgeX = min(containerEdgeFadeDst, min(pos.x - containerBoundsMin.x, containerBoundsMax.x - pos.x));
    float dstToEdgeZ = min(containerEdgeFadeDst, min(pos.z - containerBoundsMin.z, containerBoundsMax.z - pos.z));
    float edgeFade = min(dstToEdgeX, dstToEdgeZ) / containerEdgeFadeDst;

    // Calculate how high the clouds render
    float heightGradient = 0;
    float3 size = containerBoundsMax - containerBoundsMin;
    /*float2 weatherMapSamplePos = pos.zx / coverageTexTransform.w + coverageTexTransform.xy;
    float weatherMapCoverage = weatherMapTex.SampleLevel(sampler0, weatherMapSamplePos, 0).r;*/
    float heightPercent = ((pos.y - containerBoundsMin.y) / size.y) /** weatherMapCoverage*/;
    float gMin = .2;
    float gMax = .7;
    heightGradient = saturate(remap(heightPercent, 0.0, gMin, 0, 1)) * saturate(remap(heightPercent, 1, gMax, 0, 1));
    heightGradient *= edgeFade;

    // Sample the shape noise texture at the current point
    float4 shapeNoise = shapeNoiseTex.SampleLevel(sampler0, shapeSamplePos, 0);
    float4 normalisedWeights = normalize(shapeNoiseWeights);
    float shapeFBM = dot(shapeNoise, normalisedWeights) * heightGradient * (1 - heightPercent); // Multiplied by 1 - height percent to make the bottom of the clouds more wispy looking

    // Calculate the density of the shape noise
    float shapeDensity = shapeFBM - (1 - globalCoverage);

    // If the shape density is not greater than 0 there is no need to calculate the details
    if (shapeDensity > 0)
    {
        // Sample detail noise
        float3 detailSamplePos = uvw / detailNoiseTexTransform.w + detailNoiseTexTransform.xyz;
        float4 detailNoise = detailNoiseTex.SampleLevel(sampler0, detailSamplePos, 0);
        float3 normalizedDetailWeights = normalize(detailNoiseWeights.xyz);
        float detailFBM = dot(detailNoise, normalizedDetailWeights);

        // Use the density of the detail noise to erode parts of the shape noise making them more wispy around the edges
        float detailDensity = 1 - shapeDensity;
        float erosionWeight = detailDensity * detailDensity * detailDensity;
        float cloudDensity = shapeDensity - (detailFBM * erosionWeight);

        return cloudDensity * densityMultiplier;
    }
    return 0;
}

float BeersLaw(float depth)
{
    return exp(-depth);
}

float PowderEffect(float depth)
{
    return (1.0f - exp(-depth * 2.0f));
}

float BeerPowderEffect(float depth, float absorbtion, float strength = 2.0f)
{
    return strength * BeersLaw(depth * absorbtion) * PowderEffect(depth);
}

float HenyeyGreenstein(float a, float g)
{
    float g2 = g * g;
    return (1 - g2) / (4 * 3.1415 * pow(1 + g2 - 2 * g * (a), 1.5));
}

float PhaseFunction(float a)
{
    float forwardScattering = 0.857f, backScattering = 0.278f, baseBrightness = 1.f, phaseFactor = 0.385, blendFactor = .5f;
    float hgBlend = HenyeyGreenstein(a, forwardScattering) * (1 - blendFactor) + HenyeyGreenstein(a, -backScattering) * blendFactor;
    return baseBrightness + hgBlend * phaseFactor;
}

float LightMarch(float3 pos)
{
    // Calculate the distance from the current point to edge of the container in the direction of the light
    float3 dirToLight = -lightDir.xyz;
    float dstInsideBox = RayBoxDst(containerBoundsMin.xyz, containerBoundsMax.xyz, pos, dirToLight).y;

    // March to the light
    int lightSteps = lightAbsorptionData.w;
    float stepSize = densitySettings.w;
    float totalDensity = 0;
    float dstTravelled = 0;
    int stepCounter = 0;
    do
    {
        // Sample the density at the current step
        pos += dirToLight * stepSize;
        totalDensity += max(0, SampleDensity(pos) * stepSize);
        dstTravelled += stepSize;
        stepCounter++;
    } 
    while (dstTravelled < dstInsideBox && stepCounter < lightSteps); // Exit if the ray is travelling outside the box or the number of steps is past the max

    // Use the beer-powder effect to calculate the transmittance of the light as it passes through the container
    float lightAbsTowardSun = lightAbsorptionData.x;
    float cloudBrightness = lightAbsorptionData.z;
    //float transmittance = BeerPowderEffect(totalDensity, lightAbsTowardSun, 2.0f);
    float transmittance = BeersLaw(totalDensity * lightAbsTowardSun);
    return saturate(cloudBrightness + transmittance * (1 - cloudBrightness));
}

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void main( int3 id : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex, int3 groupID : SV_GroupID)
{
    // Get the UVs of the screen
    float2 resolution = float2(0, 0);
    Result.GetDimensions(resolution.x, resolution.y);
    float2 uv = (id.xy / resolution) * 2 - 1;
    uv *= float2(1, -1);

    // Only update 1/16 of the pixels per frame
    int reprojectionFrame = optimisationSettings.y;
    int bayerFilter[16] =
    {
        0,8,2,10,
        12,4,14,6,
        3,11,1,9,
        15,7,13,5
    };
    if (groupIndex != bayerFilter[reprojectionFrame])
    {
        return;
    }

    // Set the colour to the source render texture initially
    float4 col = Source[id.xy];

    // Calculate the ray origin and direction
    Ray cameraRay = CreateCameraRay(uv);
    float3 ro = cameraRay.origin;
    float3 rd = cameraRay.direction;

    // Get the ray distance information from the box
    float2 rayBoxInfo = RayBoxDst(containerBoundsMin.xyz, containerBoundsMax.xyz, ro, rd);
    float dstToBox = rayBoxInfo.x;
    float dstInsideBox = rayBoxInfo.y;

    // Phase function makes clouds brighter when looking through towards the sun
    float phaseVal = PhaseFunction(dot(rd, -lightDir));

    // Sample the blue noise to offset the distance travelled by the ray to reduce artifacting
    float blueNoiseStrength = optimisationSettings.x;
    float dstOffset = blueNoiseTex.SampleLevel(sampler0, uv, 0).r;
    dstOffset *= blueNoiseStrength;

    // Ray marching
    int numSteps = densitySettings.z;
    float dstTravelled = dstOffset;
    float transmittance = 1;
    float lightEnergy = 0;
    float lightAbsThroughCloud = lightAbsorptionData.y;
    float stepSize = densitySettings.w;
    float3 entryPoint = ro + rd * dstToBox;     // Point of intersection with the cloud container
    int stepCounter = 0;
    do
    {
        // March ray inside the box
        float3 rayPos = entryPoint + rd * dstTravelled;
        float density = SampleDensity(rayPos);

        // Only update the values if the density sampled is greater than zero
        if (density > 0)
        {
            // Add on the light energy transmittence at the current sample point
            float lightTransmittance = LightMarch(rayPos) * phaseVal;
            lightEnergy += lightTransmittance * transmittance * density * stepSize;

            // Use beers law instead of beer-powder here as beer-powder is only applied when marching towards the light
            transmittance *= BeersLaw(density * lightAbsThroughCloud * stepSize);

            // If the transmittance is very low, smapling won't effect much, so break
            if (transmittance < 0.01)
                break;
        }

        // Increment the travel distance and the step counter
        dstTravelled += stepSize;
        stepCounter++;
    } 
    while (dstTravelled < dstInsideBox && stepCounter < numSteps);  // Exit if the ray is travelling outside the box or the number of steps is past the max

    // Calculate the final colour of the clouds
    col = (col * transmittance) + (lightDiffuse * lightEnergy);
    Result[id.xy] = saturate(col);
}