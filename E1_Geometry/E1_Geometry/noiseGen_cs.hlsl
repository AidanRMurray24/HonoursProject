
#define NUM_CELLS 5
#define TOTAL_CELLS NUM_CELLS * NUM_CELLS 

Texture2D Source : register(t0);
RWTexture2D<float4> Result : register(u0);

cbuffer PointBuffer : register(b0)
{
    float4 points[TOTAL_CELLS];
};

[numthreads(8, 8, 1)]
void main(int3 groupThreadID : SV_GroupThreadID, int3 id : SV_DispatchThreadID)
{
    // Get the UVs of the texture
    float2 resolution = float2(0, 0);
    Result.GetDimensions(resolution.x, resolution.y);
    float2 uv = id / resolution;

    // Set the colour to the source render texture initially
    float4 col = Source[id.xy];
    Result[id.xy] = col;

    float minDist = 1;
    for (int i = 0; i < TOTAL_CELLS; i++)
    {
        float dist = length(normalize(points[i].xy - uv));

        if (dist < minDist)
        {
            minDist = dist;
        }
    }

    col.xyz = minDist;
    Result[id.xy] = col;
}