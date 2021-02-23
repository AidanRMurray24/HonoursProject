
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
    float4 detailNoiseTexTransform; // Offset = (x,y,z), Scale = w
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

float GetLinearDepth(float nonLinearDepth)
{
    //float nonLinearDepth = depthMap.SampleLevel(sampler0, uv, 0);
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

    return 1.0f / (_ZBufferParams.z * z + _ZBufferParams.w);
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

[numthreads(8, 8, 1)]
void main( int3 id : SV_DispatchThreadID )
{
    // Get the UVs of the screen
    float2 resolution = float2(0, 0);
    Result.GetDimensions(resolution.x, resolution.y);
    float2 uv = (float2(id.x, resolution.y - id.y) / resolution); // Get the uvs between 0 and 1 from the texture resolution
    uv = uv * 2 - 1; // Convert UVs from between 0 and 1 to between -1 and 1

    // Set the colour to the source render texture initially
    float4 col = Source[id.xy];
    Result[id.xy] = col;

    // Calculate the ray origin and direction
    Ray cameraRay = CreateCameraRay(uv);
    float3 ro = cameraRay.origin;
    float3 rd = cameraRay.direction;

    // Calculate the depth
    float3 viewVector = GetViewVector(uv);
    float depth = depthMap[id.xy];
    float linearDepth = LinearEyeDepth(depth) * length(viewVector);     // Unity method of getting linear eye depth - Got from Sebastian Lague video on clouds: https://www.youtube.com/watch?v=4QOcCGI6xOU&t=284
    //float linearDepth = GetLinearDepth(depth) * length(viewVector);   // Found on forums - Gets linear depth from near to far plane (near being 0, far being 1)

    // Get the ray distance information from the box
    float2 rayBoxInfo = RayBoxDst(containerBoundsMin.xyz, containerBoundsMax.xyz, ro, rd);
    float dstToBox = rayBoxInfo.x;
    float dstInsideBox = rayBoxInfo.y;

    // Only shade black if inside the box
    bool rayHitBox = dstInsideBox > 0 && dstToBox < linearDepth;
    if (rayHitBox)
    {
        col = 0;
        //col = shapeNoiseTex.SampleLevel(sampler0, pNear / 10, 0);
    }

    // Only show on left half of screen
    if (uv.x < 0)
    {
        //col.xyzw = depth; // Show depth
        //col.xyzw = linearDepth; // Show linear depth
    }

    //col = float4(uv, 0, 0); // Show UVs
    Result[id.xy] = col;
}