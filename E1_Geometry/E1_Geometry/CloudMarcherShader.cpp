#include "CloudMarcherShader.h"

CloudMarcherShader::CloudMarcherShader(ID3D11Device* device, HWND hwnd, int w, int h, Camera* _cam) : BaseShader(device, hwnd)
{
	uavTexAccess = NULL;
	srvTexOutput = NULL;
	screenWidth = w;
	screenHeight = h;
	cam = _cam;

	initShader(L"cloudMarcher_cs.cso", NULL);
}

CloudMarcherShader::~CloudMarcherShader()
{
}

void CloudMarcherShader::setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* sourceTexture, const XMMATRIX& projectionMatrix)
{
	// Pass the source texture and the texture to be modified to the shader
	dc->CSSetShaderResources(0, 1, &sourceTexture);
	dc->CSSetUnorderedAccessViews(0, 1, &uavTexAccess, 0);

	// Pass in buffer data
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Get the inverse of the view and projection matrices to be used in the shader
	XMMATRIX invView, invProjection;
	invView = XMMatrixInverse(nullptr, cam->getViewMatrix());
	invProjection = XMMatrixInverse(nullptr, projectionMatrix);

	// Send the information from the camera buffer to the shader
	CameraBufferType* camPtr;
	dc->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	camPtr = (CameraBufferType*)mappedResource.pData;
	camPtr->invViewMatrix = invView;
	camPtr->invProjectionMatrix = invProjection;
	camPtr->cameraPos = cam->getPosition();
	dc->Unmap(cameraBuffer, 0);
	dc->CSSetConstantBuffers(0, 1, &cameraBuffer);

	// Send the information from the container info buffer to the shader
	ContainerInfoBufferType* containerPtr;
	dc->Map(containerInfoBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	containerPtr = (ContainerInfoBufferType*)mappedResource.pData;
	containerPtr->boundsMin = XMFLOAT4(0,15,0,0);
	containerPtr->boundsMax = XMFLOAT4(100, 25, 100,0);
	dc->Unmap(containerInfoBuffer, 0);
	dc->CSSetConstantBuffers(1, 1, &containerInfoBuffer);
}

void CloudMarcherShader::createGPUViews()
{
	// Setup the description for the output texture
	D3D11_TEXTURE2D_DESC textureDesc;
	ZeroMemory(&textureDesc, sizeof(textureDesc));
	textureDesc.Width = screenWidth;
	textureDesc.Height = screenHeight;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	outputTexture = 0;
	renderer->CreateTexture2D(&textureDesc, 0, &outputTexture);

	// Setup UAV for the output texture
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(uavDesc));
	uavDesc.Format = textureDesc.Format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	renderer->CreateUnorderedAccessView(outputTexture, &uavDesc, &uavTexAccess);

	// Setup SRV for the output texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	renderer->CreateShaderResourceView(outputTexture, &srvDesc, &srvTexOutput);
}

void CloudMarcherShader::unbind(ID3D11DeviceContext* dc)
{
	ID3D11ShaderResourceView* nullSRV[] = { NULL };
	dc->CSSetShaderResources(0, 1, nullSRV);

	// Unbind output from compute shader
	ID3D11UnorderedAccessView* nullUAV[] = { NULL };
	dc->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

	// Disable Compute Shader
	dc->CSSetShader(nullptr, nullptr, 0);
}

void CloudMarcherShader::initShader(const wchar_t* cfile, const wchar_t* blank)
{
	// Load the shader and create the views
	loadComputeShader(cfile);
	createGPUViews();
	InitBuffers();
}

void CloudMarcherShader::InitBuffers()
{
	// Camera buffer
	D3D11_BUFFER_DESC camBufferDesc;
	camBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	camBufferDesc.ByteWidth = sizeof(CameraBufferType);
	camBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	camBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	camBufferDesc.MiscFlags = 0;
	camBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&camBufferDesc, NULL, &cameraBuffer);

	// Container info buffer
	D3D11_BUFFER_DESC containerBufferDesc;
	containerBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	containerBufferDesc.ByteWidth = sizeof(ContainerInfoBufferType);
	containerBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	containerBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	containerBufferDesc.MiscFlags = 0;
	containerBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&containerBufferDesc, NULL, &containerInfoBuffer);
}
