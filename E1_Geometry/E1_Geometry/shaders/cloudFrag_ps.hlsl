
// Texture and sampler registers
SamplerState Sampler0 : register(s0);
Texture2D Source : register(t0);
Texture2D depthMap : register(t1);
Texture3D noiseTex : register(t2);

cbuffer CameraBuffer : register(b0)
{
    matrix invViewMatrix;
    matrix invProjectionMatrix;
    float3 cameraPos;
    float padding;
};

cbuffer ContainerInfoBuffer : register(b1)
{
    float4 containerBoundsMin;
    float4 containerBoundsMax;
}

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 viewVector : TEXCOORD1;
};

// Returns (dstToBox, dstInsideBox). If ray misses box, dstInsideBox will be zero
float2 RayBoxDst(float3 boundsMin, float3 boundsMax, float3 rayOrigin, float3 invRaydir) {
    // Adapted from: http://jcgt.org/published/0007/03/04/
    float3 t0 = (boundsMin - rayOrigin) * invRaydir;
    float3 t1 = (boundsMax - rayOrigin) * invRaydir;
    float3 tmin = min(t0, t1);
    float3 tmax = max(t0, t1);

    float dstA = max(max(tmin.x, tmin.y), tmin.z);
    float dstB = min(tmax.x, min(tmax.y, tmax.z));

    // CASE 1: ray intersects box from outside (0 <= dstA <= dstB)
    // dstA is dst to nearest intersection, dstB dst to far intersection

    // CASE 2: ray intersects box from inside (dstA < 0 < dstB)
    // dstA is the dst to intersection behind the ray, dstB is dst to forward intersection

    // CASE 3: ray misses box (dstA > dstB)

    float dstToBox = max(0, dstA);
    float dstInsideBox = max(0, dstB - dstToBox);
    return float2(dstToBox, dstInsideBox);
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


float4 main(InputType input) : SV_TARGET
{
    // Sample the texture
    float2 uv = input.tex.xy;
    float4 col = Source.Sample(Sampler0, uv);

    // Create ray
    float viewLength = length(input.viewVector);
    float3 rayPos = cameraPos;
    float3 rayDir = input.viewVector / viewLength;

    // Calculate the distances for the container
    float2 rayToContainerInfo = RayBoxDst(containerBoundsMin, containerBoundsMax, rayPos, 1 / rayDir);
    float dstToBox = rayToContainerInfo.x;
    float dstInsideBox = rayToContainerInfo.y;

    // Calculate depth
    float nonLinearDepth = depthMap.Sample(Sampler0, uv);
    float depth = LinearEyeDepth(nonLinearDepth) * viewLength;

    bool rayHitBox = dstInsideBox > 0 && dstToBox < depth;
    if (rayHitBox)
    {
        col = 0;
    }

    return col;
}