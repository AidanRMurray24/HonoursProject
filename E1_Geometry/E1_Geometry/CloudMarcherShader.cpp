#include "CloudMarcherShader.h"

CloudMarcherShader::CloudMarcherShader(ID3D11Device* device, HWND hwnd, int w, int h, Camera* _cam, Light* _mainLight) : BaseShader(device, hwnd)
{
	uavTexAccess = NULL;
	srvTexOutput = NULL;
	screenWidth = w;
	screenHeight = h;
	cam = _cam;
	mainLight = _mainLight;
	cloudSettings.densitySettings = XMFLOAT4(0.6f, 1, 100, 0);
	cloudSettings.noiseTexTransform = XMFLOAT4(0, 0, 0, 1);
	absorptionData = XMFLOAT4(0.75f, 1.21f, 0.15f, 8.0f);

	initShader(L"cloudMarcher_cs.cso", NULL);
}

CloudMarcherShader::~CloudMarcherShader()
{
}

void CloudMarcherShader::setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* sourceTexture, ID3D11ShaderResourceView* depthMap, ID3D11ShaderResourceView* noiseTex,  const XMMATRIX& projectionMatrix, CloudContainer* container)
{
	// Pass in buffer data
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Get the inverse of the view and projection matrices to be used in the shader
	XMMATRIX invView, invProjection;
	invView = XMMatrixInverse(nullptr, cam->getViewMatrix());
	invProjection = XMMatrixInverse(nullptr, projectionMatrix);

	// Fill camera buffer
	CameraBufferType* camPtr;
	dc->Map(cameraBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	camPtr = (CameraBufferType*)mappedResource.pData;
	camPtr->invViewMatrix = invView;
	camPtr->invProjectionMatrix = invProjection;
	camPtr->cameraPos = cam->getPosition();
	dc->Unmap(cameraBuffer, 0);

	// Fill container info bufferr
	ContainerInfoBufferType* containerPtr;
	dc->Map(containerInfoBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	containerPtr = (ContainerInfoBufferType*)mappedResource.pData;
	containerPtr->boundsMin = XMFLOAT4(container->GetBoundsMin().x, container->GetBoundsMin().y, container->GetBoundsMin().z, 0);
	containerPtr->boundsMax = XMFLOAT4(container->GetBoundsMax().x, container->GetBoundsMax().y, container->GetBoundsMax().z, 0);
	dc->Unmap(containerInfoBuffer, 0);

	// Fill cloud settings buffer
	CloudSettingsBufferType* cloudSettingsPtr;
	dc->Map(cloudSettingsBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	cloudSettingsPtr = (CloudSettingsBufferType*)mappedResource.pData;
	cloudSettingsPtr->noiseTexTransform = cloudSettings.noiseTexTransform;
	cloudSettingsPtr->densitySettings = cloudSettings.densitySettings;
	dc->Unmap(cloudSettingsBuffer, 0);

	// Fill light buffer
	LightBufferType* lightPtr;
	dc->Map(lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	lightPtr = (LightBufferType*)mappedResource.pData;
	lightPtr->position = XMFLOAT4(mainLight->getPosition().x, mainLight->getPosition().y, mainLight->getPosition().z, 0);
	lightPtr->direction = XMFLOAT4(mainLight->getDirection().x, mainLight->getDirection().y, mainLight->getDirection().z, 0);
	lightPtr->colour = XMFLOAT4(mainLight->getDiffuseColour().x, mainLight->getDiffuseColour().y, mainLight->getDiffuseColour().z, 0);
	lightPtr->absorptionData = absorptionData;
	dc->Unmap(lightBuffer, 0);
	
	// Set buffer data to sahder
	dc->CSSetConstantBuffers(0, 1, &cameraBuffer);
	dc->CSSetConstantBuffers(1, 1, &containerInfoBuffer);
	dc->CSSetConstantBuffers(2, 1, &cloudSettingsBuffer);
	dc->CSSetConstantBuffers(3, 1, &lightBuffer);

	// Pass the source texture and the texture to be modified to the shader
	dc->CSSetShaderResources(0, 1, &sourceTexture);
	dc->CSSetShaderResources(1, 1, &depthMap);
	dc->CSSetShaderResources(2, 1, &noiseTex);
	dc->CSSetUnorderedAccessViews(0, 1, &uavTexAccess, 0);

	// Set the sampler inside the shader
	dc->CSSetSamplers(0, 1, &sampleState);
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
	dc->CSSetShaderResources(1, 1, nullSRV);
	dc->CSSetShaderResources(2, 1, nullSRV);

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
	// Create the constant buffers
	CreateConstantBuffer(renderer, sizeof(CameraBufferType), &cameraBuffer);
	CreateConstantBuffer(renderer, sizeof(ContainerInfoBufferType), &containerInfoBuffer);
	CreateConstantBuffer(renderer, sizeof(CloudSettingsBufferType), &cloudSettingsBuffer);
	CreateConstantBuffer(renderer, sizeof(LightBufferType), &lightBuffer);

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

void CloudMarcherShader::CreateConstantBuffer(ID3D11Device* renderer, UINT uElementSize, ID3D11Buffer** ppBufOut)
{
	D3D11_BUFFER_DESC desc;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.ByteWidth = uElementSize;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = 0;
	desc.StructureByteStride = 0;
	renderer->CreateBuffer(&desc, NULL, ppBufOut);
}
