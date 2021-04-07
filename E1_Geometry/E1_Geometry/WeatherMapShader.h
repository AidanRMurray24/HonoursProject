#pragma once
#include "../DXFramework/DXF.h"

using namespace DirectX;

class WeatherMapShader : public BaseShader
{
public:
	WeatherMapShader(ID3D11Device* device, HWND hwnd, int w, int h);
	~WeatherMapShader();

	void setShaderParameters(ID3D11DeviceContext* dc);
	void createGPUViews();
	void unbind(ID3D11DeviceContext* dc);

	// Getters
	inline ID3D11ShaderResourceView* getSRV() { return m_srvTexOutput; }

private:
	void initShader(const wchar_t* cfile, const wchar_t* blank);

	// Texture set
	ID3D11Texture2D* tex2D;
	ID3D11ShaderResourceView* m_srvTexOutput;
	ID3D11UnorderedAccessView* m_uavAccess;

	// Texture sizes
	int texWidth;
	int texHeight;

	// Gradients
	int permutationTable[512];

	// Buffers
	ID3D11Buffer* permutationBuffer;
	ID3D11ShaderResourceView* permutationBufferSRV;

	ID3D11Device* device;
};

