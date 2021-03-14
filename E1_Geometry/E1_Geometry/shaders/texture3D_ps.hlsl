// Texture pixel/fragment shader
// Basic fragment shader for rendering textured geometry

// Texture and sampler registers
Texture3D texture0 : register(t0);
SamplerState Sampler0 : register(s0);

cbuffer SliceBuffer : register(b0)
{
	float sliceVal;
	float tileVal;
	float2 padding;
};

struct InputType
{
	float4 position : SV_POSITION;
	float3 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

float invLerp(float from, float to, float value) {
	return (value - from) / (to - from);
}

float4 main(InputType input) : SV_TARGET
{
	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	return texture0.Sample(Sampler0, float3(input.tex.xy, sliceVal) * tileVal);
}