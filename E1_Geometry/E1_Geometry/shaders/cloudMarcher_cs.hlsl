
Texture2D Source : register(t0);
RWTexture2D<float4> Result : register(u0);

[numthreads(8, 8, 1)]
void main( int3 id : SV_DispatchThreadID )
{
	Result[id.xy] = 1 - Source[id.xy];
}