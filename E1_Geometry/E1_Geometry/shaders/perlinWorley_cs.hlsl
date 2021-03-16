
RWTexture3D<float4> Result : register(u0);
Texture3D<float4> perlinNoise : register(t0);
Texture3D<float4> worleyNoise : register(t1);

[numthreads(8, 8, 8)]
void main(int3 groupThreadID : SV_GroupThreadID, int3 id : SV_DispatchThreadID)
{
	float4 col = float4(0,0,0,0);

	col = perlinNoise[id.xyz];
	col.r = lerp(perlinNoise[id.xyz].r, worleyNoise[id.xyz].r, .5f);

	Result[id.xyz] = col;
}