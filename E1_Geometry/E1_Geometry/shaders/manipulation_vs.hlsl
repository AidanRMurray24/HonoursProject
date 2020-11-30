// Light vertex shader
// Standard issue vertex shader, apply matrices, pass info to pixel shader

Texture2D texture0 : register(t0);
SamplerState sampler0 : register(s0);

cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

struct InputType
{
	float4 position : POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct OutputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

OutputType main(InputType input)
{
	OutputType output;

	float maxHeight = 20.0f;
	float4 textureColor = texture0.SampleLevel(sampler0, input.tex, 0);
	input.position.y += textureColor.x * maxHeight;

	// Modify normals
	{
		float texelWidth = 1.0f / 100;
		float texelHeight = 1.0f / 100;

		textureColor = texture0.SampleLevel(sampler0, float2(input.tex.x, input.tex.y - texelHeight), 0);
		float northYpos = textureColor.x * maxHeight;
		textureColor = texture0.SampleLevel(sampler0, float2(input.tex.x + texelWidth, input.tex.y), 0);
		float eastYPos = textureColor.x * maxHeight;
		textureColor = texture0.SampleLevel(sampler0, float2(input.tex.x, input.tex.y + texelHeight), 0);
		float southYPos = textureColor.x * maxHeight;
		textureColor = texture0.SampleLevel(sampler0, float2(input.tex.x - texelWidth, input.tex.y), 0);
		float westYPos = textureColor.x * maxHeight;

		float3 northPos = float4(input.position.x, northYpos, input.position.z + 1, input.position.w);
		float3 eastPos = float4(input.position.x + 1, eastYPos, input.position.z, input.position.w);
		float3 southPos = float4(input.position.x, southYPos, input.position.z - 1, input.position.w);
		float3 westPos = float4(input.position.x - 1, westYPos, input.position.z, input.position.w);

		float3 northVec = normalize(northPos - input.position);
		float3 eastVec = normalize(eastPos - input.position);
		float3 southVec = normalize(southPos - input.position);
		float3 westVec = normalize(westPos - input.position);

		float3 northNormal = normalize(cross(northVec, eastVec));
		float3 eastNormal = normalize(cross(eastVec, southVec));
		float3 southNormal = normalize(cross(southVec, westVec));
		float3 westNormal = normalize(cross(westVec, northVec));

		float3 average = normalize((northNormal + eastNormal + southNormal + westNormal) / 4);

		input.normal.x = average.x;
		input.normal.y = average.y;
	}

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;

	// Calculate the normal vector against the world matrix only and normalise.
	output.normal = mul(input.normal, (float3x3)worldMatrix);
	output.normal = normalize(output.normal);

	return output;
}