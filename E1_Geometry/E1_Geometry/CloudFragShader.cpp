#include "CloudFragShader.h"

CloudFragShader::CloudFragShader(ID3D11Device* device, HWND hwnd, int w, int h, Camera* _cam) : BaseShader(device, hwnd)
{
	screenWidth = w;
	screenHeight = h;
	cam = _cam;

	initShader(L"cloudFrag_vs.cso", L"cloudFrag_ps.cso");
}

CloudFragShader::~CloudFragShader()
{
}

void CloudFragShader::setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* sourceTexture, ID3D11ShaderResourceView* depthMap, ID3D11ShaderResourceView* noiseTex, const XMMATRIX& worldMatrix, const XMMATRIX& projectionMatrix, CloudContainer* container)
{
	// Pass the source texture and the texture to be modified to the shader
	dc->PSSetShaderResources(0, 1, &sourceTexture);
	dc->PSSetShaderResources(1, 1, &depthMap);
	dc->PSSetShaderResources(2, 1, &noiseTex);

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
	dc->PSSetConstantBuffers(0, 1, &cameraBuffer);

	// Send the information from the container info buffer to the shader
	ContainerInfoBufferType* containerPtr;
	dc->Map(containerInfoBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	containerPtr = (ContainerInfoBufferType*)mappedResource.pData;
	containerPtr->boundsMin = XMFLOAT4(container->GetBoundsMin().x, container->GetBoundsMin().y, container->GetBoundsMin().z, 0);
	containerPtr->boundsMax = XMFLOAT4(container->GetBoundsMax().x, container->GetBoundsMax().y, container->GetBoundsMax().z, 0);
	dc->Unmap(containerInfoBuffer, 0);
	dc->PSSetConstantBuffers(1, 1, &containerInfoBuffer);

	dc->PSSetSamplers(0, 1, &sampleState);

	// Transpose the matrices to prepare them for the shader.
	XMMATRIX tworld, tview, tproj;
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(cam->getOrthoViewMatrix());
	tproj = XMMatrixTranspose(projectionMatrix);

	// Send matrix data
	MatrixBufferType* dataPtr;
	dc->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	dc->Unmap(matrixBuffer, 0);
	dc->VSSetConstantBuffers(0, 1, &matrixBuffer);
}

void CloudFragShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{
	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);
	InitBuffers();
}

void CloudFragShader::InitBuffers()
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

	// Setup the matrix buffer
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	// Create a texture sampler state description.
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	renderer->CreateSamplerState(&samplerDesc, &sampleState);
}
