#include "SimpleRayMarcherShader.h"

SimpleRayMarcherShader::SimpleRayMarcherShader(ID3D11Device* device, HWND hwnd, int w, int h, Camera* cam, Light* _light) : BaseShader(device, hwnd)
{
	light = _light;
	camera = cam;
	deviceObject = device;
	sWidth = w;
	sHeight = h;
	initShader(L"simpleRayMarcher_cs.cso", NULL);
}

SimpleRayMarcherShader::~SimpleRayMarcherShader()
{
}

void SimpleRayMarcherShader::setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* texture1, const XMMATRIX&projectionMatrix)
{
	// Pass the source texture and the texture to be modified to the shader
	dc->CSSetShaderResources(0, 1, &texture1);
	dc->CSSetUnorderedAccessViews(0, 1, &m_uavAccess, 0);

	// Create a mapped resource object to map the data from the buffers to and pass them into the shader
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Get the inverse of the view and projection matrices to be used in the shader
	XMMATRIX invView, invProjection;
	invView = XMMatrixInverse(nullptr, camera->getViewMatrix());
	invProjection = XMMatrixInverse(nullptr, projectionMatrix);

	// Send the information from the camera buffer to the shader
	CameraBufferType* camPtr;
	dc->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	camPtr = (CameraBufferType*)mappedResource.pData;
	camPtr->invViewMatrix = invView;
	camPtr->invProjectionMatrix = invProjection;
	camPtr->cameraPos = camera->getPosition();
	dc->Unmap(cameraBuffer, 0);
	dc->CSSetConstantBuffers(0, 1, &cameraBuffer);

	// Send the information from the light buffer to the shader
	LightBufferType* lightPtr;
	dc->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;
	lightPtr->position = XMFLOAT4(light->getPosition().x, light->getPosition().y, light->getPosition().z, 0);
	lightPtr->direction = XMFLOAT4(light->getDirection().x, light->getDirection().y, light->getDirection().z, 0);
	lightPtr->colour = XMFLOAT4(light->getDiffuseColour().x, light->getDiffuseColour().y, light->getDiffuseColour().z, 0);
	lightPtr->intensityTypeAndAngle = XMFLOAT4(1.0f, 1.0f, 0.0f, 0.0f);
	dc->Unmap(lightBuffer, 0);
	dc->CSSetConstantBuffers(1, 1, &lightBuffer);
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

	// Init camera buffer
	D3D11_BUFFER_DESC camBufferDesc;
	camBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	camBufferDesc.ByteWidth = sizeof(CameraBufferType);
	camBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	camBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	camBufferDesc.MiscFlags = 0;
	camBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&camBufferDesc, NULL, &cameraBuffer);

	// Init light buffer
	D3D11_BUFFER_DESC lightBufferDesc;
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&lightBufferDesc, NULL, &lightBuffer);
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
