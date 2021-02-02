
RWTexture3D<float4> Result : register(u0);
StructuredBuffer<float4> pointsA : register(t0);
StructuredBuffer<float4> pointsB : register(t1);
StructuredBuffer<float4> pointsC : register(t2);

cbuffer WorleyBuffer : register(b0)
{
    float4 numCells;
    float3 padding;
    float noisePersistence = .5f;
};

static const int3 cellOffsets[] =
{
    // Centre
    int3(0,0,0),

    // Front face
    int3(0,0,1),
    int3(-1,1,1),
    int3(-1,0,1),
    int3(-1,-1,1),
    int3(0,1,1),
    int3(0,-1,1),
    int3(1,1,1),
    int3(1,0,1),
    int3(1,-1,1),

    // Back face
    int3(0,0,-1),
    int3(-1,1,-1),
    int3(-1,0,-1),
    int3(-1,-1,-1),
    int3(0,1,-1),
    int3(0,-1,-1),
    int3(1,1,-1),
    int3(1,0,-1),
    int3(1,-1,-1),

    // Ring around centre
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

float GenerateWorleyNoise(StructuredBuffer<float4> points, int numCells, float3 uvw)
{
    // Find which cell the current pixel occupies
    int3 cellID = floor(uvw * numCells);

    // Loop through the adjacent cells to find the closest point to the pixel
    float minSqrDist = 1;
    for (int i = 0; i < 27; i++)
    {
        // Calculate the current adjacent cell to check
        int3 adjCellID = cellID + cellOffsets[i];

        // If the cell is outside the texture, wrap around to the other side
        if (minComponent(adjCellID) == -1 || maxComponent(adjCellID) == numCells)
        {
            int3 wrappedID = (adjCellID + numCells) % (uint3)numCells;
            int adjCellIndex = wrappedID.x + numCells * (wrappedID.y + wrappedID.z * numCells);
            float3 wrappedPoint = points[adjCellIndex];

            for (int wrapOffsetIndex = 0; wrapOffsetIndex < 27; wrapOffsetIndex++)
            {
                float3 dirVector = uvw - (wrappedPoint + cellOffsets[wrapOffsetIndex]);
                minSqrDist = min(minSqrDist, dot(dirVector, dirVector));
            }
        }
        else
        {
            int adjCellIndex = adjCellID.x + numCells * (adjCellID.y + adjCellID.z * numCells);
            float3 dirVector = uvw - points[adjCellIndex];
            minSqrDist = min(minSqrDist, dot(dirVector, dirVector));
        }
    }

    return sqrt(minSqrDist);
}

[numthreads(8, 8, 8)]
void main(int3 groupThreadID : SV_GroupThreadID, int3 id : SV_DispatchThreadID)
{
    // Get the UVWs of the texture
    float3 resolution = float3(0, 0, 0);
    Result.GetDimensions(resolution.x, resolution.y, resolution.z);
    float3 uvw = id / (float)resolution;

    // Set the colour to the source render texture initially
    float4 col = float4(0,0,0,0);
    Result[id.xyz] = col;

    // Calculate the maximum distance by getting the distance across the diagonal of a cell
    float cellWidth = 1 / (numCells.x + 1);
    float3 cellSize = float3(cellWidth, cellWidth, cellWidth);
    float maxDist = sqrt(dot(cellSize, cellSize));

    // Calculate each worley layer
    float layerA = GenerateWorleyNoise(pointsA, numCells.x, uvw);
    float layerB = GenerateWorleyNoise(pointsB, numCells.y, uvw);
    float layerC = GenerateWorleyNoise(pointsC, numCells.z, uvw);

    float noiseSum = layerA + (layerB * noisePersistence) + (layerC * noisePersistence * noisePersistence);
    float maxVal = 1 + (noisePersistence) + (noisePersistence * noisePersistence);

    col.xyz = (noiseSum / maxVal) / maxDist;

    Result[id.xyz] = 1 - col;
}