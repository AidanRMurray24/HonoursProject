#pragma once
#include "../DXFramework/DXF.h"

using namespace std;
using namespace DirectX;

class PerlinNoiseShader : public BaseShader
{
public:
	PerlinNoiseShader(ID3D11Device* _device, HWND hwnd, int w, int h, int d);
	~PerlinNoiseShader();

	void setShaderParameters(ID3D11DeviceContext* dc);
	void createGPUViews();
	void unbind(ID3D11DeviceContext* dc);

	// Getters
	inline ID3D11ShaderResourceView* getSRV() { return m_srvTexOutput; }

private:
	void initShader(const wchar_t* cfile, const wchar_t* blank);

	// Texture set
	ID3D11Texture3D* tex3D;
	ID3D11ShaderResourceView* m_srvTexOutput;
	ID3D11UnorderedAccessView* m_uavAccess;

	// Texture sizes
	int texWidth;
	int texHeight;
	int texDepth;

	// Gradients
	int permutationTable[512];

	// Buffers
	ID3D11Buffer* permutationBuffer;
	ID3D11ShaderResourceView* permutationBufferSRV;

	ID3D11Device* device;
};

