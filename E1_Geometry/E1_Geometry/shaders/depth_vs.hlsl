Texture2D heightMapTexture : register(t0);
SamplerState sampler0 : register(s0);

cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

cbuffer HeightMapBuffer : register(b1)
{
	float4 heightMapSettings; // x = hasHeightMap?, y = height
}

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float4 depthPosition : TEXCOORD0;
};

OutputType main(InputType input)
{
    OutputType output;

	// Calculate height map if there is one
	if (heightMapSettings.x > 0)
	{
		float4 textureColor = heightMapTexture.SampleLevel(sampler0, input.tex, 0);
		input.position.y += textureColor.x * heightMapSettings.y;
	}

    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(input.position, worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Store the position value in a second input value for depth value calculations.
    output.depthPosition = output.position;
	
    return output;
}