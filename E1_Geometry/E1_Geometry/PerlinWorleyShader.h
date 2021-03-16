#pragma once
#include "../DXFramework/DXF.h"

using namespace std;
using namespace DirectX;

class PerlinWorleyShader : public BaseShader
{
public:
	PerlinWorleyShader(ID3D11Device* device, HWND hwnd, int w, int h, int d);
	~PerlinWorleyShader();

	void setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* perlinNoiseTexture, ID3D11ShaderResourceView* worleyNoiseTexture);
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

	ID3D11Device* device;
};

