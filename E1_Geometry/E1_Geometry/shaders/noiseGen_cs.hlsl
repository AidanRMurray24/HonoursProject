
#define NUM_CELLS 5
#define TOTAL_CELLS NUM_CELLS * NUM_CELLS 

Texture2D Source : register(t0);
RWTexture2D<float4> Result : register(u0);

cbuffer PointBuffer : register(b0)
{
    float4 points[TOTAL_CELLS];
    float4 cellInfo; // x = Num cells, y = Total Cells, z = Cell size
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

    // Find the closest point to the current texture pixel
    float minSqrDist = 1;
    for (int i = 0; i < cellInfo.y; i++)
    {
        float2 dirVector = points[i].xy - uv;
        minSqrDist = min(minSqrDist, dot(dirVector, dirVector));
    }

    // Calculate the maximum distance by getting the distance across the diagonal of a cell
    float2 cellSize = float2(cellInfo.z, cellInfo.z);
    float maxDist = sqrt(dot(cellSize, cellSize));

    // Normalise the distance by dividing it by the maximum distance
    col.xyz = sqrt(minSqrDist) / maxDist;
    Result[id.xy] = 1 - col;
}