#pragma once
#include "../DXFramework/DXF.h"

#define NUM_CELLS 5
#define TOTAL_CELLS NUM_CELLS * NUM_CELLS * NUM_CELLS

using namespace std;
using namespace DirectX;

class NoiseGeneratorShader : public BaseShader
{
private:
	struct PointBufferType
	{
		XMFLOAT4 points[TOTAL_CELLS];
		XMFLOAT4 cellInfo; // x = Num cells, y = Total Cells, z = Cell size, w = tile value
	};

public:
	NoiseGeneratorShader(ID3D11Device* device, HWND hwnd, int w, int h, int d);
	~NoiseGeneratorShader();

	void setShaderParameters(ID3D11DeviceContext* dc, float tileVal);
	void createOutputUAV();
	void unbind(ID3D11DeviceContext* dc);
	inline ID3D11ShaderResourceView* getSRV() { return m_srvTexOutput; }

private:
	void initShader(const wchar_t* cfile, const wchar_t* blank);
	void GenerateWorleyNoisePoints();

	// Texture set
	ID3D11Texture2D* tex2D;
	ID3D11Texture3D* tex3D;
	ID3D11ShaderResourceView* m_srvTexOutput;
	ID3D11UnorderedAccessView* m_uavAccess;

	// Buffers
	ID3D11Buffer* pointBuffer;

	// Random Points
	std::vector<XMFLOAT3> points;
	int pointsSeed;

	// Texture sizes
	int texWidth;
	int texHeight;
	int texDepth;

	ID3D11Device* device;
};

