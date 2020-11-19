#pragma once
#include "../DXFramework/DXF.h"

#define NUM_CELLS 5
#define TOTAL_CELLS NUM_CELLS * NUM_CELLS

using namespace std;
using namespace DirectX;

class NoiseGeneratorShader : public BaseShader
{
private:
	struct PointBufferType
	{
		XMFLOAT4 points[TOTAL_CELLS];
	};

public:
	NoiseGeneratorShader(ID3D11Device* device, HWND hwnd, int w, int h);
	~NoiseGeneratorShader();

	void setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* texture1);
	void createOutputUAV();
	ID3D11ShaderResourceView* getSRV() { return m_srvTexOutput; };
	void unbind(ID3D11DeviceContext* dc);

private:
	void initShader(const wchar_t* cfile, const wchar_t* blank);
	void GenerateWorleyNoisePoints();

	ID3D11ShaderResourceView* srv;
	ID3D11UnorderedAccessView* uav;

	// texture set
	ID3D11Texture2D* m_tex;
	ID3D11UnorderedAccessView* m_uavAccess;
	ID3D11ShaderResourceView* m_srvTexOutput;

	int sWidth;
	int sHeight;

	ID3D11Buffer* pointBuffer;
	std::vector<XMFLOAT2> points;
};

