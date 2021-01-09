#pragma once
#include "../DXFramework/DXF.h"

using namespace DirectX;

class CloudMarcherShader : public BaseShader
{
public:
	CloudMarcherShader(ID3D11Device* device, HWND hwnd, int _screenWidth, int _screenHeight);
	~CloudMarcherShader();

	void setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* sourceTexture);
	void createOutputUAV();
	void unbind(ID3D11DeviceContext* dc);
	inline ID3D11ShaderResourceView* getSRV() { return srvTexOutput; }

private:
	void initShader(const wchar_t* cfile, const wchar_t* blank);

	ID3D11ShaderResourceView* srvTexOutput;
	ID3D11UnorderedAccessView* uavTexAccess;

	int screenWidth;
	int screenHeight;
};

