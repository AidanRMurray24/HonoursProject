#define MAX_STEPS 100
#define MAX_DIST 100
#define SURF_DIST 1e-3

Texture2D Source : register(t0);
RWTexture2D<float4> Result : register(u0);

cbuffer CameraBuffer : register(b0)
{
    matrix _CameraToWorld, _CameraInverseProjection;
    float3 _WorldSpaceCameraPos;
    float padding;
};

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
    float3 origin = mul(_CameraToWorld, float4(0, 0, 0, 1)).xyz;
    float3 direction = mul(_CameraInverseProjection, float4(uv, 0, 1)).xyz;
    direction = mul(_CameraToWorld, float4(direction, 0)).xyz;
    direction = normalize(direction);
    return CreateRay(origin, direction);
}

float GetDist(float3 p)
{
    float4 sphere = float4(0,1,6,1);

    float sphereDist = length(p - sphere.xyz) - sphere.w;

    float d = sphereDist;
    return d;
}

float RayMarch(float3 ro, float3 rd)
{
    float dO = 0.0f;
    for (int i = 0; i < MAX_STEPS; i++)
    {
        float3 p = ro + dO * rd;
        float dS = GetDist(p);
        dO += dS;
        if (dS < SURF_DIST || dO > MAX_DIST) break;
    }
    return dO;
}

float3 GetNormal(float3 p)
{
    float d = GetDist(p);
    float2 e = float2(.01, 0);

    float3 n = d - float3
    (
        GetDist(p - e.xyy),
        GetDist(p - e.yxy),
        GetDist(p - e.yyx)
    );

    return normalize(n);
}

float GetLight(float3 p)
{
    float3 lightPos = float3(0, 5, 6);
    float3 l = normalize(lightPos - p);
    float3 n = GetNormal(p);

    float dif = clamp(dot(n, l), 0., 1.);
    return dif;
}

[numthreads(8, 8, 1)]
void main(int3 groupThreadID : SV_GroupThreadID, int3 id : SV_DispatchThreadID)
{
    // Get the UVs of the screen
    float2 resolution = float2(0,0);
    Result.GetDimensions(resolution.x, resolution.y);
    float2 uv = (id.xy -.5f * resolution.xy) / resolution.y * float2(1, -1);

    // Set the colour to the source render texture initially
    float4 col = Source[id.xy];
    Result[id.xy] = col;

    // Calculate the ray origin and direction
    //Ray cameraRay = CreateCameraRay(uv);
    //float3 ro = cameraRay.origin;
    //float3 rd = cameraRay.direction;
    float3 ro = float3(0, 1, 0);
    float3 rd = normalize(float3(uv.x, uv.y, 1));

    // March the ray from the origin in the ray direction and retrieve the distance to the closest object hit
    float d = RayMarch(ro, rd);

    // If the distance was greater than or equal to the max dist then nothing was hit so return
    if (d >= MAX_DIST)
        return;

    // Get the point of intersection
    float3 p = ro + rd * d;

    // Calculate lighting
    float dif = GetLight(p);
    col.rgb = dif;

    Result[id.xy] = col;
}