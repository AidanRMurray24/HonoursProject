#define MAX_STEPS 100
#define MAX_DIST 100
#define SURF_DIST 1e-3

Texture2D Source : register(t0);
RWTexture2D<float4> Result : register(u0);

cbuffer CameraBuffer : register(b0)
{
    matrix invViewMatrix, invProjectionMatrix;
    float3 cameraPos;
    float padding;
};

cbuffer LightBuffer : register(b1)
{
    float4 lightPos;
    float4 lightDir;
    float4 lightDiff;
    float4 lightIntensityTypeAndAngle; // x = intensity, y = type, z = angle
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
    float3 origin = mul(invViewMatrix, float4(0, 0, 0, 1)).xyz;
    float3 direction = mul(invProjectionMatrix, float4(uv, 0, 1)).xyz;
    direction = mul(invViewMatrix, float4(direction, 0)).xyz;
    direction = normalize(direction);
    return CreateRay(origin, direction);
}

float GetDist(float3 p)
{
    // Create a sphere at position 0,0,0 with a radius of 1
    float4 sphere = float4(0,10,0,1);

    // Calculate the distance to the sphere using the current ray marched point
    float sphereDist = length(p - sphere.xyz) - sphere.w;

    // Set the final distance to the sphere distance as there are no other objects for now
    float d = sphereDist;
    return d;
}

float RayMarch(float3 ro, float3 rd)
{
    // Iterate for a max number of steps
    float dO = 0.0f;
    for (int i = 0; i < MAX_STEPS; i++)
    {
        // March the ray into the scene
        float3 p = ro + dO * rd;
        float dS = GetDist(p);
        dO += dS;

        // If the distance to scene is smaller than surface value or
        // If the distance from the origin in greater than the max distance set then break
        if (dS < SURF_DIST || dO > MAX_DIST) break;
    }

    // Return the distance the ray traveled from the origin
    return dO;
}

float3 GetNormal(float3 p)
{
    // Get the distance to the scene at the point
    float d = GetDist(p);
    float2 epsilon = float2(.01f, 0); // Very small amount to move

    // Calculate the normal by sampling the distance to the scene slighly over in the x, y and z and finding the slope
    float3 n = d - float3
    (
        GetDist(p - epsilon.xyy),
        GetDist(p - epsilon.yxy),
        GetDist(p - epsilon.yyx)
    );

    // Noramilise the normal vector before returning it
    return normalize(n);
}

float3 CalculateDirectionalLight(float3 p)
{
    float3 normal = GetNormal(p);

    // Calculate light falloff and colour
    float lightFallOff = max(0, dot(-lightDir, normal) * lightIntensityTypeAndAngle.x);
    float3 lightColor = lightDiff.xyz * lightFallOff;

    return lightColor;
}

float3 CalculatePointLight(float3 p)
{
    float3 normal = GetNormal(p);

    // Calculate light falloff and colour
    float3 lightVec = normalize(lightPos - p);
    float lightFallOff = max(0, dot(lightVec, normal) * lightIntensityTypeAndAngle.x);
    float3 lightColor = lightDiff.xyz * lightFallOff;

    return lightColor;
}

float3 CalculateSpotLight(float3 p)
{
    float3 normal = GetNormal(p);

    float3 lightVec = normalize(lightPos - p);
    float cutOff = saturate(dot(-lightVec, lightDir));

    float angle = lightIntensityTypeAndAngle.z / 2 * 3.1415f / 180.f;
    if (cutOff < cos(angle))
        return 0;

    float lightFallOff = max(0, dot(lightVec, normal) * lightIntensityTypeAndAngle.x);
    float3 lightColor = lightDiff.xyz * lightFallOff;

    return lightColor;
}

float GetLight(float3 p)
{
    float3 finalColor = 0;
    float3 ambientLight = float3(0.2, 0.2, 0.2);

    // Get the light type
    switch (lightIntensityTypeAndAngle.y)
    {
    case 0: // SPOT
    {
        finalColor = CalculateSpotLight(p);
        break;
    }
    case 1: // DIRECTIONAL
    {
        finalColor = CalculateDirectionalLight(p);
        break;
    }
    case 2: // POINT
    {
        finalColor = CalculatePointLight(p);
        break;
    }
    }

    return finalColor + ambientLight;
}

[numthreads(8, 8, 1)]
void main(int3 groupThreadID : SV_GroupThreadID, int3 id : SV_DispatchThreadID)
{
    // Get the UVs of the screen
    float2 resolution = float2(0,0);
    Result.GetDimensions(resolution.x, resolution.y);
    float2 uv = (id.xy / float2(resolution.x, resolution.y) * 2 - 1) * float2(1, -1);

    // Set the colour to the source render texture initially
    float4 col = Source[id.xy];
    Result[id.xy] = col;

    // Calculate the ray origin and direction
    Ray cameraRay = CreateCameraRay(uv);
    float3 ro = cameraRay.origin;
    float3 rd = cameraRay.direction;

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