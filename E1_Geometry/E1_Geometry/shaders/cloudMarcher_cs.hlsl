#define NUM_THREADS 4

SamplerState sampler0 : register(s0);
RWTexture2D<float4> Result : register(u0);
Texture2D<float4> Source : register(t0);
Texture3D<float4> shapeNoiseTex : register(t1);
Texture3D<float4> detailNoiseTex : register(t2);
Texture2D<float4> weatherMapTex : register(t3);
Texture2D<float4> blueNoiseTex : register(t4);
Texture2D<float4> previousFrameTex : register(t5);

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
    float4 optimisationSettings; // Blue noise strength = x, reprojectionFrame = y, useTemporalReprojection = z
}

cbuffer LightBuffer : register(b3)
{
    float4 lightDir;
    float4 lightPos;
    float4 lightDiffuse;
    float4 lightAbsorptionData; // Absorption to sun = x, Absorption through cloud = y, Cloud Brightness = z, Marching steps = w
    float4 inOutScatterSettings; // inScatter = x, outScatter = y, inOutScatterBlend = z
    float4 silverLiningIntensityAndExponent; // silver lining intensity = x, silver lining exponent = y
}

cbuffer WeatherMapBuffer : register(b4)
{
    float4 coverageTexTransform; // Offset = (x,y,z), Scale = w
    float4 heightTexTransform; // Offset = (x,y,z), Scale = w
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

float ReMap(float v, float minOld, float maxOld, float minNew, float maxNew)
{
    return minNew + (v - minOld) * (maxNew - minNew) / (maxOld - minOld);
}

float GetBlueNoiseOffset(float2 uv, float2 resolution)
{
    // Sample the blue noise to offset the distance travelled by the ray to reduce artifacting
    float blueNoiseStrength = optimisationSettings.x;
    float dstOffset = blueNoiseTex.SampleLevel(sampler0, uv * resolution / 500.0f, 0).r;
    dstOffset *= blueNoiseStrength;
    return dstOffset;
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

    // Sample the weather map for the coverage and height at the current pos
    float2 coverageSamplePos = pos.zx / coverageTexTransform.w + coverageTexTransform.xy;
    float4 coverageSample = weatherMapTex.SampleLevel(sampler0, coverageSamplePos, 0) * weatherMapIntensities.r;
    float2 heightSamplePos = pos.zx / heightTexTransform.w + heightTexTransform.xy;
    float4 heightSample = weatherMapTex.SampleLevel(sampler0, heightSamplePos, 0) * weatherMapIntensities.g;

    // Calculate how high the clouds render
    float heightGradient = 0;
    float3 size = containerBoundsMax - containerBoundsMin;
    float heightPercent = ((pos.y - containerBoundsMin.y) / size.y);
    float gMin = .2;
    float gMax = saturate(gMin + heightSample.g);
    heightGradient = saturate(ReMap(heightPercent, 0.0, gMin, 0, 1)) * saturate(ReMap(heightPercent, 1, gMax, 0, 1));
    heightGradient *= edgeFade;

    // Sample the shape noise texture at the current point
    float4 shapeNoise = shapeNoiseTex.SampleLevel(sampler0, shapeSamplePos, 0);
    float4 normalisedWeights = normalize(shapeNoiseWeights);
    float shapeFBM = dot(shapeNoise, normalisedWeights) * heightGradient * (1 - heightPercent); // Multiplied by 1 - height percent to make the bottom of the clouds more wispy looking

    // Calculate the density of the shape noise
    float coverage = 1 - (globalCoverage + coverageSample.r);
    float shapeDensity = saturate(shapeFBM - coverage);

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

float HenyeyGreenstein(float cosAngle, float g)
{
    float g2 = g * g;
    return (1 - g2) / (4 * 3.1415 * pow(1 + g2 - 2 * g * cosAngle, 1.5));
}

// Adapted from Haggstrom (2018) http://www.diva-portal.org/smash/get/diva2:1223894/FULLTEXT01.pdf
float InOutScatter(float cosAngle)
{
    // Get values from buffers
    float inScatter = inOutScatterSettings.x;
    float outScatter = inOutScatterSettings.y;
    float inOutBlend = inOutScatterSettings.z;
    float silverLiningIntensity = silverLiningIntensityAndExponent.x;
    float silverLiningExponent = silverLiningIntensityAndExponent.y;

    // Calculate the in scatter probability 
    float firstHG = HenyeyGreenstein(cosAngle, inScatter);
    float secondHG = silverLiningIntensity * pow(saturate(cosAngle), silverLiningExponent); // For adding a more centralised intensity around the sun

    // Get the highest intensity value for the in scattering
    float inScatterHG = max(firstHG, secondHG);

    // Out scatter is then calculated using the Henyey Greenstein function with the negitive value for the out scatter as out scatter bounces back in the direction of the light source.
    float outScatterHG = HenyeyGreenstein(cosAngle, -outScatter);

    // Return the interpolated value between the scatter values to strike a balance between how much light is scattered in and out from the clouds
    return lerp(inScatterHG, outScatterHG, inOutBlend);
}

float LightMarch(float3 pos, float blueNoiseOffset)
{
    // Calculate the distance from the current point to edge of the container in the direction of the light
    float3 dirToLight = -lightDir.xyz;
    float dstInsideBox = RayBoxDst(containerBoundsMin.xyz, containerBoundsMax.xyz, pos, dirToLight).y;

    // March to the light
    int lightSteps = lightAbsorptionData.w;
    float stepSize = densitySettings.w;
    float densityToSun = 0;
    float dstTravelled = blueNoiseOffset;
    for (int i = 0; i < lightSteps; i++)
    {
        // Sample the density at the current step
        pos += dirToLight * stepSize;
        densityToSun += max(0, saturate(SampleDensity(pos) * stepSize));
        dstTravelled += stepSize;

        // Break early if ray has travelled outside of the box
        if (dstTravelled > dstInsideBox)
            break;
    } 

    // Use the beer-powder effect to calculate the transmittance of the light as it passes through the container
    float lightAbsTowardSun = lightAbsorptionData.x;
    float cloudBrightness = lightAbsorptionData.z;
    float transmittance = BeersLaw(densityToSun * lightAbsTowardSun);
    return saturate(transmittance * cloudBrightness);
}

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void main( int3 id : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex, int3 groupID : SV_GroupID)
{
    // Check if temproal reprojection has been enabled first
    bool useTemporalReprojection = optimisationSettings.z;
    if (useTemporalReprojection)
    {
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
            return;
    }

    // Get the UVs of the screen
    float2 resolution = float2(0, 0);
    Result.GetDimensions(resolution.x, resolution.y);
    float2 uv = id.xy / resolution;

    // Set the colour to the source render texture initially
    float4 col = Source.SampleLevel(sampler0, uv, 0);

    // Convert UVs to have (0,0) at the centre of the screen
    uv = uv * 2 - 1;
    uv *= float2(1, -1);

    // Calculate the ray origin and direction
    Ray cameraRay = CreateCameraRay(uv);
    float3 ro = cameraRay.origin;
    float3 rd = cameraRay.direction;

    // Get the ray distance information from the box
    float2 rayBoxInfo = RayBoxDst(containerBoundsMin.xyz, containerBoundsMax.xyz, ro, rd);
    float dstToBox = rayBoxInfo.x;
    float dstInsideBox = rayBoxInfo.y;

    // Phase function that makes clouds brighter when looking through towards the sun
    float cosAngle = dot(rd, -lightDir);
    float phaseVal = InOutScatter(cosAngle);

    // Blue noise which can be used to offset the marching ray's travel distance to reduce artifacting with large step values
    float blueNoiseOffset = GetBlueNoiseOffset(uv, resolution);

    // Get values from buffers
    int numSteps = densitySettings.z;
    float stepSize = densitySettings.w;
    float lightAbsThroughCloud = lightAbsorptionData.y;

    // Ray marching
    float dstTravelled = blueNoiseOffset;
    float lightExtinction = 1;
    float lightEnergy = 0;
    float3 startPos = ro + rd * dstToBox; // First point of intersection with the cloud container
    for (int i = 0; i < numSteps; i++)
    {
        // March ray inside the box
        float3 rayPos = startPos + rd * dstTravelled;
        float density = SampleDensity(rayPos);

        // Only update the values if the density sampled is greater than zero
        if (density > 0.0f)
        {
            // Add on the light energy transmittence at the current sample point
            float lightTransmittance = LightMarch(rayPos, blueNoiseOffset) + phaseVal;
            lightEnergy += lightTransmittance * lightExtinction * density * stepSize;

            // Calculate the extinction of light at this density point using beer's law
            lightExtinction *= BeersLaw(density * lightAbsThroughCloud * stepSize);

            // If the lightExtinction is very low, sampling won't effect much, so break
            if (lightExtinction < 0.01f)
                break;
        }

        // Increment the travel distance
        dstTravelled += stepSize;

        // Break early if ray has travelled outside of the box
        if (dstTravelled > dstInsideBox)
            break;
    } 

    // Calculate the final colour of the clouds
    col = (col * lightExtinction) + (lightDiffuse * lightEnergy);
    Result[id.xy] = saturate(col);
}