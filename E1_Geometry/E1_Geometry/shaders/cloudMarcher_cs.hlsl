
SamplerState sampler0 : register(s0);
Texture2D Source : register(t0);
Texture2D depthMap : register(t1);
RWTexture2D<float4> Result : register(u0);

float cloudScale = 1;
float cloudOffset = 1;

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

struct Ray
{
    float3 origin;
    float3 direction;
};

Ray CreateRay(float3 origin, float3 direction) {
    Ray ray;
    ray.origin = origin;
    ray.direction = direction;
    return ray;
}

Ray CreateCameraRay(float2 uv) {
    float3 origin = mul(invViewMatrix, float4(0, 0, 0, 1)).xyz;
    float3 direction = mul(invProjectionMatrix, float4(uv, 0, 1)).xyz;
    direction = mul(invViewMatrix, float4(direction, 0)).xyz;
    direction = normalize(direction);
    return CreateRay(origin, direction);
}

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
    float near = 0.1f;
    float far = 1000.f;
    float4 zBufferParams;
    zBufferParams.x = 1.0f - (far / near);
    zBufferParams.y = far / near;
    zBufferParams.z = zBufferParams.x / far;
    zBufferParams.w = zBufferParams.y / far;
    return 1.0f / (zBufferParams.z * z + zBufferParams.w);
}

float SampleDesity(float3 position)
{
    //float3 uvw = position * 
    return float3(0,0,0);
}

[numthreads(8, 8, 1)]
void main( int3 id : SV_DispatchThreadID )
{
    // Get the UVs of the screen
    float2 resolution = float2(0, 0);
    Result.GetDimensions(resolution.x, resolution.y);
    float2 uv = (id.xy / resolution * 2 - 1) * float2(1, -1);

    // Set the colour to the source render texture initially
    float4 col = Source[id.xy];
    Result[id.xy] = col;

    // Calculate the ray origin and direction
    Ray cameraRay = CreateCameraRay(uv);
    float3 ro = cameraRay.origin;
    float3 rd = cameraRay.direction;

    // Get the ray distance information from the box
    float2 rayBoxInfo = RayBoxDst(containerBoundsMin.xyz, containerBoundsMax.xyz, ro, 1/rd);
    float dstToBox = rayBoxInfo.x;
    float dstInsideBox = rayBoxInfo.y;

    // Calculate the depth
    float3 viewVector = mul(invProjectionMatrix, float4(uv, 0, 1)).xyz;
    viewVector = mul(invViewMatrix, float4(viewVector, 0)).xyz;
    float nonLinearDepth = depthMap[id.xy].x;
    float depth = LinearEyeDepth(nonLinearDepth) * length(viewVector);

    // Only shade black if inside the box
    bool rayHitBox = dstInsideBox > 0 && dstToBox < depth;
    if (rayHitBox)
    {
        col = 0;
    }

    Result[id.xy] = col;
    //Result[id.xy] = float4(depth, depth, depth,1);
}