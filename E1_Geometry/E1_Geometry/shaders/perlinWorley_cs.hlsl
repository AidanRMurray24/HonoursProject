
RWTexture3D<float4> Result : register(u0);
Texture3D<float4> perlinNoise : register(t0);
Texture3D<float4> worleyNoise : register(t1);

[numthreads(8, 8, 8)]
void main(int3 groupThreadID : SV_GroupThreadID, int3 id : SV_DispatchThreadID)
{
	float4 col = float4(0,0,0,0);
	float4 worley = (worleyNoise[id.xyz] - .5f) * 1.5f;
	float perlin = perlinNoise[id.xyz].r;
	col = worleyNoise[id.xyz];
	col.r = lerp(perlin, worley.r, 0.2f);

	Result[id.xyz] = col;
}