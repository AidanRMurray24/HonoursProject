// texture vertex shader
// Basic shader for rendering textured geometry

cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
};

cbuffer CameraBuffer : register(b1)
{
	matrix invViewMatrix;
	matrix invProjectionMatrix;
	float3 cameraPos;
	float padding;
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
	float3 viewVector : TEXCOORD1;
};

OutputType main(InputType input)
{
	OutputType output;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	output.position = mul(input.position, worldMatrix);
	output.position = mul(output.position, viewMatrix);
	output.position = mul(output.position, projectionMatrix);

	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;

    output.normal = input.normal;

	// Calculate the view vector
	//float3 viewVector = mul(invProjectionMatrix, float4(float2(input.tex.x * 2 - 1, (1-input.tex.y) * 2 - 1), 0, 1));
	float3 viewVector = mul(invProjectionMatrix, float4(float2(input.tex.x , 1-input.tex.y) * 2 - 1, 0, 1));
	output.viewVector = mul(invViewMatrix, float4(viewVector, 0));

	return output;
}