#include "TemporalReprojectionShader.h"
#include "BufferCreationHelper.h"

TemporalReprojectionShader::TemporalReprojectionShader(ID3D11Device* device, HWND hwnd, int _screenWidth, int _screenHeight) : BaseShader(device, hwnd)
{
	uavTexAccess = NULL;
	srvTexOutput = NULL;
	screenWidth = _screenWidth;
	screenHeight = _screenHeight;

	initShader(L"temporalReprojection_cs.cso", NULL);
}

TemporalReprojectionShader::~TemporalReprojectionShader()
{
}

void TemporalReprojectionShader::setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* currentFrameTex, ID3D11ShaderResourceView* previousFrameTex, XMMATRIX oldViewProjMatrix, XMMATRIX currentInvViewProjMatrix, int currentFrame)
{
	// Pass in buffer data
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Fill info buffer
	ReprojectionInfoBufferType* infoPtr;
	dc->Map(infoBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	infoPtr = (ReprojectionInfoBufferType*)mappedResource.pData;
	infoPtr->currentFrame = currentFrame;
	infoPtr->currentInvViewProjMatrix = currentInvViewProjMatrix;
	infoPtr->oldViewProjMatrix = oldViewProjMatrix;
	dc->Unmap(infoBuffer, 0);

	// Set buffer data to shader
	dc->CSSetConstantBuffers(0, 1, &infoBuffer);

	// Pass the textures to the shader
	dc->CSSetShaderResources(0, 1, &currentFrameTex);
	dc->CSSetShaderResources(1, 1, &previousFrameTex);
	dc->CSSetUnorderedAccessViews(0, 1, &uavTexAccess, 0);

	// Set the sampler inside the shader
	dc->CSSetSamplers(0, 1, &sampleState);
}

void TemporalReprojectionShader::createGPUViews()
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

void TemporalReprojectionShader::unbind(ID3D11DeviceContext* dc)
{
	ID3D11ShaderResourceView* nullSRV[] = { NULL };
	dc->CSSetShaderResources(0, 1, nullSRV);
	dc->CSSetShaderResources(1, 1, nullSRV);

	// Unbind output from compute shader
	ID3D11UnorderedAccessView* nullUAV[] = { NULL };
	dc->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

	// Disable Compute Shader
	dc->CSSetShader(nullptr, nullptr, 0);
}

void TemporalReprojectionShader::initShader(const wchar_t* cfile, const wchar_t* blank)
{
	// Load the shader and create the views
	loadComputeShader(cfile);
	createGPUViews();
	InitBuffers();
}

void TemporalReprojectionShader::InitBuffers()
{
	// Create the constant buffers
	BufferCreationHelper::CreateConstantBuffer(renderer, sizeof(ReprojectionInfoBufferType), &infoBuffer);

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
