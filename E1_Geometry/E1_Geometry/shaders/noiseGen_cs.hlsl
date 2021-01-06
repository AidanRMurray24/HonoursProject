
#define NUM_CELLS 5
#define TOTAL_CELLS NUM_CELLS * NUM_CELLS 

RWTexture2D<float4> Result : register(u0);

cbuffer PointBuffer : register(b0)
{
    float4 points[TOTAL_CELLS];
    float4 cellInfo; // x = Num cells, y = Total Cells, z = Cell size, w = tile value
};

static const int2 cellOffsets[] =
{
    // Centre
    int2(0,0),

    // Ring around centre
    int2(-1, -1),
    int2(0, -1),
    int2(1, -1),
    int2(-1, 0),
    int2(1, 0),
    int2(-1, 1),
    int2(0, 1),
    int2(1, 1)
};

float maxComponent(float2 vec) {
    return max(vec.x, vec.y);
}

float minComponent(float2 vec) {
    return min(vec.x, vec.y);
}

[numthreads(8, 8, 1)]
void main(int3 groupThreadID : SV_GroupThreadID, int3 id : SV_DispatchThreadID)
{
    // Get the UVs of the texture
    float2 resolution = float2(0, 0);
    Result.GetDimensions(resolution.x, resolution.y);
    float2 uv = id / (float)resolution;

    // Set the colour to the source render texture initially
    float4 col = float4(0,0,0,0);
    Result[id.xy] = col;

    // Make the texture tile
    uv = (uv * cellInfo.w) % 1;

    // Find which cell the current pixel occupies
    int2 cellID = floor(uv * NUM_CELLS);

    // Loop through the adjacent cells to find the closest point to the pixel
    float minSqrDist = 1;
    for (int i = 0; i < 9; i++)
    {
        // Calculate the current adjacent cell to check
        int2 adjCellID = cellID + cellOffsets[i];

        // If the cell is outside the texture, wrap around to the other side
        if (minComponent(adjCellID) == -1 || maxComponent(adjCellID) == NUM_CELLS)
        {
            int2 wrappedID = (adjCellID + NUM_CELLS) % NUM_CELLS;
            int adjCellIndex = wrappedID.x + (wrappedID.y * NUM_CELLS);
            float2 wrappedPoint = points[adjCellIndex];

            for (int wrapOffsetIndex = 0; wrapOffsetIndex < 9; wrapOffsetIndex++)
            {
                float2 dirVector = uv - (wrappedPoint + cellOffsets[wrapOffsetIndex]);
                minSqrDist = min(minSqrDist, dot(dirVector, dirVector));
            }
        }
        else
        {
            int adjCellIndex = adjCellID.x + (adjCellID.y * NUM_CELLS);
            float2 dirVector = uv - points[adjCellIndex];
            minSqrDist = min(minSqrDist, dot(dirVector, dirVector));
        }
    }

    // Calculate the maximum distance by getting the distance across the diagonal of a cell
    float2 cellSize = float2(cellInfo.z, cellInfo.z);
    float maxDist = sqrt(dot(cellSize, cellSize));

    // Normalise the distance by dividing it by the maximum distance
    col.xyz = sqrt(minSqrDist) / maxDist;

    Result[id.xy] = 1 - col;
}