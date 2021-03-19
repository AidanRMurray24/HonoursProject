
RWTexture3D<float4> Result : register(u0);
StructuredBuffer<int> permutation : register(t0);

float2 CosineInterpolation(float2 uv)
{
	return ((6 * uv - 15) * uv + 10)* uv* uv* uv;
}

float Random21(float2 uv)
{
	return frac(sin(uv.x * 100.f + uv.y * 6574.f) * 6647.f);
}

float2 Random2DVector(float2 st) 
{
	st = float2(dot(st, float2(127.1, 311.7)), dot(st, float2(269.5, 183.3)));
	return -1.0 + 2.0 * frac(sin(st) * 43758.5453123);
}

float3 Random3DVector(float3 uvw)
{
	uvw = float3(dot(uvw, float3(127.1, 311.7, 658.4)), dot(uvw, float3(269.5, 183.3, 54.8)), dot(uvw, float3(347.1, 561.7, 898.4)));
	return -1.0 + 2.0 * frac(sin(uvw) * 43758.5453123);
}

float ValueNoise2D(float2 uv)
{
	float2 localUV = frac(uv);				// Get the local UVs of the current cell
	localUV = CosineInterpolation(localUV);	// Smooths the interpolation between cells
	float2 gridCellID = floor(uv);			// Get the current cell ID

	// Generate a random value based on the intersections of the grid
	float bottomLeft = Random21(gridCellID);
	float bottomRight = Random21(gridCellID + float2(1, 0));
	float topLeft = Random21(gridCellID + float2(0, 1));
	float topRight = Random21(gridCellID + float2(1, 1));

	// Perform the bilinear interpolation between each corner value
	float bottomLerp = lerp(bottomLeft, bottomRight, localUV.x);
	float topLerp = lerp(topLeft, topRight, localUV.x);
	float finalLerp = lerp(bottomLerp, topLerp, localUV.y);

	return finalLerp;
}

float PerlinNoise2D(float2 uv)
{
	float2 i = floor(uv);
	float2 f = frac(uv);

	// Fade lerp values
	float2 lerpFactor = f * f * (3.0 - 2.0 * f);

	// Get grid point positions
	float2 bL = i + float2(0.0, 0.0);
	float2 bR = i + float2(1.0, 0.0);
	float2 tL = i + float2(0.0, 1.0);
	float2 tR = i + float2(1.0, 1.0);

	// Get the displacement from the point in the cell to each corner of the cell
	float2 dispBL = f - float2(0.0, 0.0);
	float2 dispBR = f - float2(1.0, 0.0);
	float2 dispTL = f - float2(0.0, 1.0);
	float2 dispTR = f - float2(1.0, 1.0);

	// Dot between the gradients and the displacement vector
	float dotBL = dot(Random2DVector(bL), dispBL);
	float dotBR = dot(Random2DVector(bR), dispBR);
	float dotTL = dot(Random2DVector(tL), dispTL);
	float dotTR = dot(Random2DVector(tR), dispTR);

	// Bilinear interpolation to get the final value
	float bottomLerp = lerp(dotBL, dotBR, lerpFactor.x);
	float topLerp = lerp(dotTL, dotTR, lerpFactor.x);
	float finalLerp = lerp(bottomLerp, topLerp, lerpFactor.y);

	return finalLerp;
}

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

double fade(double t) {
	// Fade function as defined by Ken Perlin.  This eases coordinate values
	// so that they will "ease" towards integral values.  This ends up smoothing
	// the final output.
	return t * t * t * (t * (t * 6 - 15) + 10);			// 6t^5 - 15t^4 + 10t^3
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
	double x1, x2, y1, y2;
	x1 = lerp(grad(aaa, xf, yf, zf), grad(baa, xf - 1, yf, zf), u);
	x2 = lerp(grad(aba, xf, yf - 1, zf), grad(bba, xf - 1, yf - 1, zf), u);
	y1 = lerp(x1, x2, v);

	x1 = lerp(grad(aab, xf, yf, zf - 1), grad(bab, xf - 1, yf, zf - 1), u);
	x2 = lerp(grad(abb, xf, yf - 1, zf - 1), grad(bbb, xf - 1, yf - 1, zf - 1), u);
	y2 = lerp(x1, x2, v);

	return (lerp(y1, y2, w) + 1) / 2; // For convenience we bind the result to 0 - 1 (theoretical min/max before is [-1, 1])
}

[numthreads(8, 8, 8)]
void main(int3 groupThreadID : SV_GroupThreadID, int3 id : SV_DispatchThreadID)
{
	// Get the UVWs of the texture
	float3 resolution = float3(0, 0, 0);
	Result.GetDimensions(resolution.x, resolution.y, resolution.z);
	float3 uvw = id / (float)resolution;

	int numOctaves = 8;
	float lacunarity = 2.0f;
	float gain = 0.5f;
	float persistence = 1;
	float maxValue = 0;
	double noise = 0;
	float frequency = lacunarity;
	for (int i = 0; i < numOctaves; i++)
	{
		noise += Perlin3D(uvw * frequency, frequency) * persistence;
		maxValue += persistence;
		frequency *= lacunarity;
		persistence *= gain;
	}
	noise /= maxValue;

	float4 colour = float4(noise, noise, noise, 0);
	Result[id] = colour;
}