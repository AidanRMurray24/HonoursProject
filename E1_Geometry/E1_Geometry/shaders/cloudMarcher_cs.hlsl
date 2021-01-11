
Texture2D Source : register(t0);
RWTexture2D<float4> Result : register(u0);

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

[numthreads(8, 8, 1)]
void main( int3 id : SV_DispatchThreadID )
{
    // Get the UVs of the screen
    float2 resolution = float2(0, 0);
    Result.GetDimensions(resolution.x, resolution.y);
    float2 uv = (id.xy / float2(resolution.x, resolution.y) * 2 - 1) * float2(1, -1);

    // Set the colour to the source render texture initially
    float4 col = Source[id.xy];
    Result[id.xy] = col;

    // Calculate the ray origin and direction
    Ray cameraRay = CreateCameraRay(uv);
    float3 ro = cameraRay.origin;
    float3 rd = cameraRay.direction;

    float2 rayBoxInfo = RayBoxDst(containerBoundsMin.xyz, containerBoundsMax.xyz, ro, 1/rd);
    float dstToBox = rayBoxInfo.x;
    float dstInsideBox = rayBoxInfo.y;

    bool rayHitBox = dstInsideBox > 0;
    if (rayHitBox)
    {
        col = 0;
    }

    Result[id.xy] = ro.xyzz;
    Result[id.xy] = col;
}