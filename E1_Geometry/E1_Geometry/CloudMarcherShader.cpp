#include "CloudMarcherShader.h"

CloudMarcherShader::CloudMarcherShader(ID3D11Device* device, HWND hwnd, int _screenWidth, int _screenHeight) : BaseShader(device, hwnd)
{
	uavTexAccess = NULL;
	srvTexOutput = NULL;
	screenWidth = _screenWidth;
	screenHeight = _screenHeight;

	initShader(L"cloudMarcher_cs.cso", NULL);
}

CloudMarcherShader::~CloudMarcherShader()
{
}

void CloudMarcherShader::setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* sourceTexture)
{
	// Pass the source texture and the texture to be modified to the shader
	dc->CSSetShaderResources(0, 1, &sourceTexture);
	dc->CSSetUnorderedAccessViews(0, 1, &uavTexAccess, 0);
}

void CloudMarcherShader::createOutputUAV()
{

}

void CloudMarcherShader::unbind(ID3D11DeviceContext* dc)
{
}

void CloudMarcherShader::initShader(const wchar_t* cfile, const wchar_t* blank)
{
}
