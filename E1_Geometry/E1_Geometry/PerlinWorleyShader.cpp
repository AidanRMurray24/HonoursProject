#include "PerlinWorleyShader.h"

PerlinWorleyShader::PerlinWorleyShader(ID3D11Device* _device, HWND hwnd, int w, int h, int d) : BaseShader(_device, hwnd)
{
	device = _device;
	texWidth = w;
	texHeight = h;
	texDepth = d;

	initShader(L"perlinWorley_cs.cso", NULL);
}

PerlinWorleyShader::~PerlinWorleyShader()
{
}

void PerlinWorleyShader::setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* perlinNoiseTexture, ID3D11ShaderResourceView* worleyNoiseTexture)
{
	dc->CSSetUnorderedAccessViews(0, 1, &m_uavAccess, 0);
	dc->CSSetShaderResources(0, 1, &perlinNoiseTexture);
	dc->CSSetShaderResources(1, 1, &worleyNoiseTexture);
}

void PerlinWorleyShader::createGPUViews()
{
	// Setup the description for the 3D texture
	D3D11_TEXTURE3D_DESC textureDesc3D;
	ZeroMemory(&textureDesc3D, sizeof(textureDesc3D));
	textureDesc3D.Width = texWidth;
	textureDesc3D.Height = texHeight;
	textureDesc3D.Depth = texDepth;
	textureDesc3D.MipLevels = 1;
	textureDesc3D.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc3D.Usage = D3D11_USAGE_DEFAULT;
	textureDesc3D.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	textureDesc3D.CPUAccessFlags = 0;
	textureDesc3D.MiscFlags = 0;
	tex3D = 0;
	renderer->CreateTexture3D(&textureDesc3D, 0, &tex3D);

	// Setup UAV for 3D texture
	D3D11_UNORDERED_ACCESS_VIEW_DESC descUAV;
	ZeroMemory(&descUAV, sizeof(descUAV));
	descUAV.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // DXGI_FORMAT_UNKNOWN;
	descUAV.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE3D;
	descUAV.Texture3D.MipSlice = 0;
	descUAV.Texture3D.FirstWSlice = 0;
	descUAV.Texture3D.WSize = texDepth;
	renderer->CreateUnorderedAccessView(tex3D, &descUAV, &m_uavAccess);

	// Setup SRV for 3D texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = textureDesc3D.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE3D;
	srvDesc.Texture3D.MostDetailedMip = 0;
	srvDesc.Texture3D.MipLevels = 1;
	renderer->CreateShaderResourceView(tex3D, &srvDesc, &m_srvTexOutput);
}

void PerlinWorleyShader::unbind(ID3D11DeviceContext* dc)
{
	ID3D11ShaderResourceView* nullSRV[] = { NULL };
	dc->CSSetShaderResources(0, 1, nullSRV);
	dc->CSSetShaderResources(1, 1, nullSRV);

	// Unbind output from compute shader
	ID3D11UnorderedAccessView* nullUAV[] = { NULL };
	dc->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

	ID3D11Buffer* nullConstBuffer[] = { NULL };
	dc->CSSetConstantBuffers(0, 1, nullConstBuffer);

	// Disable Compute Shader
	dc->CSSetShader(nullptr, nullptr, 0);
}

void PerlinWorleyShader::initShader(const wchar_t* cfile, const wchar_t* blank)
{
	// Load the shader and create the views
	loadComputeShader(cfile);
	createGPUViews();
}
