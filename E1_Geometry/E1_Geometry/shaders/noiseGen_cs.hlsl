
#define NUM_CELLS 5
#define TOTAL_CELLS NUM_CELLS * NUM_CELLS * NUM_CELLS

RWTexture3D<float4> Result : register(u0);

cbuffer PointBuffer : register(b0)
{
    float4 points[TOTAL_CELLS];
    float4 cellInfo; // x = Num cells, y = Total Cells, z = Cell size, w = tile value
};

static const int3 cellOffsets[] =
{
    // centre
    int3(0,0,0),
    // front face
    int3(0,0,1),
    int3(-1,1,1),
    int3(-1,0,1),
    int3(-1,-1,1),
    int3(0,1,1),
    int3(0,-1,1),
    int3(1,1,1),
    int3(1,0,1),
    int3(1,-1,1),
    // back face
    int3(0,0,-1),
    int3(-1,1,-1),
    int3(-1,0,-1),
    int3(-1,-1,-1),
    int3(0,1,-1),
    int3(0,-1,-1),
    int3(1,1,-1),
    int3(1,0,-1),
    int3(1,-1,-1),
    // ring around centre
    int3(-1,1,0),
    int3(-1,0,0),
    int3(-1,-1,0),
    int3(0,1,0),
    int3(0,-1,0),
    int3(1,1,0),
    int3(1,0,0),
    int3(1,-1,0)
};

float maxComponent(float3 vec) {
    return max(vec.x, max(vec.y, vec.z));
}

float minComponent(float3 vec) {
    return min(vec.x, min(vec.y, vec.z));
}

[numthreads(8, 8, 8)]
void main(int3 groupThreadID : SV_GroupThreadID, int3 id : SV_DispatchThreadID)
{
    // Get the UVs of the texture
    float3 resolution = float3(0, 0, 0);
    Result.GetDimensions(resolution.x, resolution.y, resolution.z);
    float3 uv = id / (float)resolution;

    // Set the colour to the source render texture initially
    float4 col = float4(0,0,0,0);
    Result[id.xyz] = col;

    // Make the texture tile
    uv = (uv * cellInfo.w) % 1;

    // Find which cell the current pixel occupies
    int3 cellID = floor(uv * NUM_CELLS);

    // Loop through the adjacent cells to find the closest point to the pixel
    float minSqrDist = 1;
    for (int i = 0; i < 27; i++)
    {
        // Calculate the current adjacent cell to check
        int3 adjCellID = cellID + cellOffsets[i];

        // If the cell is outside the texture, wrap around to the other side
        if (minComponent(adjCellID) == -1 || maxComponent(adjCellID) == NUM_CELLS)
        {
            int3 wrappedID = (adjCellID + NUM_CELLS) % (uint3)NUM_CELLS;
            int adjCellIndex = wrappedID.x + NUM_CELLS * (wrappedID.y + wrappedID.z * NUM_CELLS);
            float3 wrappedPoint = points[adjCellIndex];

            for (int wrapOffsetIndex = 0; wrapOffsetIndex < 27; wrapOffsetIndex++)
            {
                float3 dirVector = uv - (wrappedPoint + cellOffsets[wrapOffsetIndex]);
                minSqrDist = min(minSqrDist, dot(dirVector, dirVector));
            }
        }
        else
        {
            int adjCellIndex = adjCellID.x + NUM_CELLS * (adjCellID.y + adjCellID.z * NUM_CELLS);
            float3 dirVector = uv - points[adjCellIndex];
            minSqrDist = min(minSqrDist, dot(dirVector, dirVector));
        }
    }

    // Calculate the maximum distance by getting the distance across the diagonal of a cell
    float3 cellSize = float3(cellInfo.z, cellInfo.z, cellInfo.z);
    float maxDist = sqrt(dot(cellSize, cellSize));

    // Normalise the distance by dividing it by the maximum distance
    col.xyz = sqrt(minSqrDist) / maxDist;

    Result[id.xyz] = 1 - col;
    //Result[id.xyz] = float4(1,1,1,1);
}