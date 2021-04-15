#define NUM_THREADS 4

SamplerState sampler0 : register(s0);
Texture2D<float4> currentFrameTex : register(t0);
Texture2D<float4> previousFrameTex : register(t1);
RWTexture2D<float4> Result : register(u0);

cbuffer InfoBuffer : register(b0)
{
	float4x4 oldviewProjMatrix, currentInvViewProj;
	float currentFrame;
	float3 padding;
};

[numthreads(NUM_THREADS, NUM_THREADS, 1)]
void main(int3 id : SV_DispatchThreadID, uint groupIndex : SV_GroupIndex, int3 groupID : SV_GroupID)
{
    Result[id.xy] = currentFrameTex[id.xy];
   
    // Only update 1/16 of the pixels per frame
    int bayerFilter[16] =
    {
        0,8,2,10,
        12,4,14,6,
        3,11,1,9,
        15,7,13,5
    };
    int renderedPixelGroupIndex = bayerFilter[(int)currentFrame];
    if (groupIndex == renderedPixelGroupIndex)
    {
        return;
    }

    // Get the UVs of the screen
    double2 resolution = double2(0, 0);
    Result.GetDimensions(resolution.x, resolution.y);
    double2 uv = (id.xy / resolution) * 2.0f - 1.0f;

    // Convert the the max depth point at this current uv coord to world space
    double4 depthPos = double4(uv, 1.0f, 1.0f); // float.z is 1.0 as it is assumed to be at max depth
    double4 worldPos = mul(depthPos, currentInvViewProj);
    worldPos /= worldPos.w;

    // Convert the current world space coord back to screen space using the view-projection matrix from the last frame
    double4 previousPos = mul(worldPos, oldviewProjMatrix);
    previousPos /= previousPos.w;
    double2 oldUV = previousPos.xy * 0.5f + 0.5f;
    
    // Set to black if trying to sample from edge of screen
    if (oldUV.x < 0.0f || oldUV.x > 1.0f || oldUV.y < 0.0f || oldUV.y > 1.0f)
    {
        //// Calculate the dispatch ID for this threads current generated pixel
        //uint3 groupThreadID = uint3((uint)((float)renderedPixelGroupIndex % NUM_THREADS), (uint)((float)renderedPixelGroupIndex / NUM_THREADS), 0);
        //uint3 dispatchID = (groupID * uint3(NUM_THREADS, NUM_THREADS, 1) + groupThreadID);

        Result[id.xy] = float4(0, 0, 0, 0);
        return;
    }

    float4 colour = previousFrameTex.SampleLevel(sampler0, oldUV, 0);

    Result[id.xy] = colour;
}