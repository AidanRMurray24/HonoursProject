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
    float2 resolution = float2(0, 0);
    Result.GetDimensions(resolution.x, resolution.y);
    float2 uv = (id.xy / resolution) * 2.0f - 1.0f;

    // Convert the the max depth point at this current uv coord to world space
    float4 depthPos = float4(uv, 1.0f, 1.0f); // float.z is 1.0 as it is assumed to be at max depth
    float4 worldPos = mul(depthPos, currentInvViewProj);
    worldPos /= worldPos.w;

    // Convert the current world space coord back to screen space using the view-projection matrix from the last frame
    float4 previousPos = mul(worldPos, oldviewProjMatrix);
    previousPos /= previousPos.w;
    float2 oldUV = previousPos.xy * 0.5f + 0.5f;

    uv = id.xy / resolution;
    float2 motionVec = oldUV - uv;
    float motionVecLength = length(motionVec);
    float motionVecN = normalize(motionVec);

    int numSteps = 5;
    float stepSize = motionVecLength / (float)numSteps;
    float4 colourSample = 0;
    for (int i = 0; i < numSteps; i++)
    {
        // Sample a point along the motion vector
        float2 samplePoint = uv + (motionVecN * i * stepSize);

        // Clamp values between 0 and 1
        samplePoint.x = max(min(samplePoint.x, 1), 0);
        samplePoint.y = max(min(samplePoint.y, 1), 0);

        // Use the sample point to sample the previous frame
        colourSample += previousFrameTex.SampleLevel(sampler0, samplePoint, 0);
    }
    colourSample /= numSteps;

    Result[id.xy] = colourSample;
}