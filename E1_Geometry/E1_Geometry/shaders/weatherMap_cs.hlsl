
RWTexture2D<float4> Result : register(u0);
StructuredBuffer<int> permutation : register(t0);

double grad(int hash, double x, double y, double z)
{
	switch (hash & 0xF)
	{
	case 0x0: return  x + y;
	case 0x1: return -x + y;
	case 0x2: return  x - y;
	case 0x3: return -x - y;
	case 0x4: return  x + z;
	case 0x5: return -x + z;
	case 0x6: return  x - z;
	case 0x7: return -x - z;
	case 0x8: return  y + z;
	case 0x9: return -y + z;
	case 0xA: return  y - z;
	case 0xB: return -y - z;
	case 0xC: return  y + x;
	case 0xD: return -y + z;
	case 0xE: return  y - x;
	case 0xF: return -y - z;
	default: return 0; // never happens
	}
}

double fade(double t) 
{
	return t * t * t * (t * (t * 6 - 15) + 10);
}

int inc(int num, int size)
{
	num++;
	if (size > 0) num %= size;

	return num;
}

double Perlin3D(float3 uvw, int size)
{
	float x = uvw.x;
	float y = uvw.y;
	float z = uvw.z;

	if (size > 0) {									// If we have any repeat on, change the coordinates to their "local" repetitions
		x = x % size;
		y = y % size;
		z = z % size;
	}

	int xi = (int)x % size;								// Calculate the "unit cube" that the point asked will be located in
	int yi = (int)y % size;								// The left bound is ( |_x_|,|_y_|,|_z_| ) and the right bound is that
	int zi = (int)z % size;								// plus 1.  Next we calculate the location (from 0.0 to 1.0) in that cube.
	double xf = x - (int)x;								// We also fade the location to smooth the result.
	double yf = y - (int)y;
	double zf = z - (int)z;
	double u = fade(xf);
	double v = fade(yf);
	double w = fade(zf);

	// Get hash numbers at all 8 corners
	int aaa, aba, aab, abb, baa, bba, bab, bbb;
	aaa = permutation[permutation[permutation[xi] + yi] + zi];
	aba = permutation[permutation[permutation[xi] + inc(yi, size)] + zi];
	aab = permutation[permutation[permutation[xi] + yi] + inc(zi, size)];
	abb = permutation[permutation[permutation[xi] + inc(yi, size)] + inc(zi, size)];
	baa = permutation[permutation[permutation[inc(xi, size)] + yi] + zi];
	bba = permutation[permutation[permutation[inc(xi, size)] + inc(yi, size)] + zi];
	bab = permutation[permutation[permutation[inc(xi, size)] + yi] + inc(zi, size)];
	bbb = permutation[permutation[permutation[inc(xi, size)] + inc(yi, size)] + inc(zi, size)];

	// Trilinear interpolation
	double x1, x2, y1, y2, finalLerp;
	x1 = lerp(grad(aaa, xf, yf, zf), grad(baa, xf - 1, yf, zf), u);
	x2 = lerp(grad(aba, xf, yf - 1, zf), grad(bba, xf - 1, yf - 1, zf), u);
	y1 = lerp(x1, x2, v);

	x1 = lerp(grad(aab, xf, yf, zf - 1), grad(bab, xf - 1, yf, zf - 1), u);
	x2 = lerp(grad(abb, xf, yf - 1, zf - 1), grad(bbb, xf - 1, yf - 1, zf - 1), u);
	y2 = lerp(x1, x2, v);

	finalLerp = lerp(y1, y2, w);

	return (finalLerp + 1) / 2; // For convenience we bind the result to 0 - 1 (theoretical min/max before is [-1, 1])
}

float LayeredPerlinNoise(float2 uv, float initialFrequency, int octaves, float lacunarity, float gain)
{
	float persistence = 1;
	float maxValue = 0;
	double noise = 0;
	for (int i = 0; i < octaves; i++)
	{
		noise += Perlin3D(float3(uv, 0) * initialFrequency, initialFrequency) * persistence;
		maxValue += persistence;
		initialFrequency *= lacunarity;
		persistence *= gain;
	}
	noise /= maxValue;

	return noise;
}

[numthreads(8, 8, 1)]
void main(int3 id : SV_DispatchThreadID)
{
	// Get the UVs of the texture
	float2 resolution = float2(0, 0);
	Result.GetDimensions(resolution.x, resolution.y);
	float2 uv = id / (float)resolution;

	float coverageNoise = LayeredPerlinNoise(uv, 8, 8, 2, 0.5f);
	float cloudTypeNoise = LayeredPerlinNoise(uv, 16, 4, 2, 0.5f);

	// Subtract each of the noise values from each other to give a more varied noise pattern
	float4 colour = float4(coverageNoise - cloudTypeNoise, cloudTypeNoise - coverageNoise, 0, 0) + .1f;
	Result[id.xy] = saturate(colour);
}