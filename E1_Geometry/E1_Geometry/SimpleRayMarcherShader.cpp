#include "SimpleRayMarcherShader.h"

SimpleRayMarcherShader::SimpleRayMarcherShader(ID3D11Device* device, HWND hwnd, int w, int h, Camera* cam) : BaseShader(device, hwnd)
{
	camera = cam;
	deviceObject = device;
	sWidth = w;
	sHeight = h;
	initShader(L"simpleRayMarcher_cs.cso", NULL);
}

SimpleRayMarcherShader::~SimpleRayMarcherShader()
{
}

void SimpleRayMarcherShader::setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* texture1)
{
	dc->CSSetShaderResources(0, 1, &texture1);
	dc->CSSetUnorderedAccessViews(0, 1, &m_uavAccess, 0);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	CameraBufferType* camPtr;
	dc->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	camPtr = (CameraBufferType*)mappedResource.pData;
	camPtr->_WorldSpaceCameraPos = camera->getPosition();
	dc->Unmap(cameraBuffer, 0);
	dc->CSSetConstantBuffers(0, 1, &cameraBuffer);
}

void SimpleRayMarcherShader::createOutputUAV()
{
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = sWidth;
	textureDesc.Height = sHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	m_tex = 0;
	renderer->CreateTexture2D(&textureDesc, 0, &m_tex);

	// Output texture
	D3D11_UNORDERED_ACCESS_VIEW_DESC descUAV;
	ZeroMemory(&descUAV, sizeof(descUAV));
	descUAV.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; ;// DXGI_FORMAT_UNKNOWN;
	descUAV.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	descUAV.Texture2D.MipSlice = 0;
	renderer->CreateUnorderedAccessView(m_tex, &descUAV, &m_uavAccess);

	// Source texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	renderer->CreateShaderResourceView(m_tex, &srvDesc, &m_srvTexOutput);

	// Camera buffer
	D3D11_BUFFER_DESC camBufferDesc;
	camBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	camBufferDesc.ByteWidth = sizeof(CameraBufferType);
	camBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	camBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	camBufferDesc.MiscFlags = 0;
	camBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&camBufferDesc, NULL, &cameraBuffer);
}

void SimpleRayMarcherShader::unbind(ID3D11DeviceContext* dc)
{
	ID3D11ShaderResourceView* nullSRV[] = { NULL };
	dc->CSSetShaderResources(0, 1, nullSRV);

	// Unbind output from compute shader
	ID3D11UnorderedAccessView* nullUAV[] = { NULL };
	dc->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

	// Disable Compute Shader
	dc->CSSetShader(nullptr, nullptr, 0);
}

void SimpleRayMarcherShader::initShader(const wchar_t* cfile, const wchar_t* blank)
{
	loadComputeShader(cfile);
	createOutputUAV();
}

HRESULT SimpleRayMarcherShader::CreateStructuredBuffer(ID3D11Device* pDevice, UINT uElementSize, UINT uCount, void* pInitData, ID3D11Buffer** ppBufOut)
{
	*ppBufOut = nullptr;
	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
	desc.ByteWidth = uElementSize * uCount;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = uElementSize;
	if (pInitData) 
	{ 
		D3D11_SUBRESOURCE_DATA InitData;
		InitData.pSysMem = pInitData;
		return pDevice->CreateBuffer(&desc, &InitData, ppBufOut);
	}
	else
		return pDevice->CreateBuffer(&desc, nullptr, ppBufOut);
}
