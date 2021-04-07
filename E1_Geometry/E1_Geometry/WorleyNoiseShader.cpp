#include "WorleyNoiseShader.h"
#include <stdlib.h>
#include <time.h>
#include "BufferCreationHelper.h"

WorleyNoiseShader::WorleyNoiseShader(ID3D11Device* _device, HWND hwnd, int w, int h, int d) : BaseShader(_device, hwnd)
{
	pointsABuffer = nullptr;
	pointsABufferSRV = nullptr;

	device = _device;
	texWidth = w;
	texHeight = h;
	texDepth = d;

	initShader(L"worleyNoise_cs.cso", NULL);
}

WorleyNoiseShader::~WorleyNoiseShader()
{
}

void WorleyNoiseShader::setShaderParameters(ID3D11DeviceContext* dc, TextureChannel channel)
{
	WorleyNoiseSettings* settings = nullptr;

	switch (channel)
	{
	case TextureChannel::RED:
		settings = &redChannelSettings;
		break;
	case TextureChannel::GREEN:
		settings = &greenChannelSettings;
		break;
	case TextureChannel::BLUE:
		settings = &blueChannelSettings;
		break;
	case TextureChannel::ALPHA:
		settings = &alphaChannelSettings;
		break;
	default:
		settings = &redChannelSettings;
		break;
	}

	settings->pointsA = GenerateWorleyNoisePoints(settings->seed, settings->numCellsA);
	settings->pointsB = GenerateWorleyNoisePoints(settings->seed, settings->numCellsB);
	settings->pointsC = GenerateWorleyNoisePoints(settings->seed, settings->numCellsC);

	// Fill in point data
	XMFLOAT4* pointsA = new XMFLOAT4[settings->pointsA.size()];
	for (size_t i = 0; i < settings->pointsA.size(); i++)
		pointsA[i] = settings->pointsA[i];

	// Fill in point data
	XMFLOAT4* pointsB = new XMFLOAT4[settings->pointsB.size()];
	for (size_t i = 0; i < settings->pointsB.size(); i++)
		pointsB[i] = settings->pointsB[i];

	// Fill in point data
	XMFLOAT4* pointsC = new XMFLOAT4[settings->pointsC.size()];
	for (size_t i = 0; i < settings->pointsC.size(); i++)
		pointsC[i] = settings->pointsC[i];

	// Create structured buffers
	BufferCreationHelper::CreateStructuredBuffer(renderer, sizeof(XMFLOAT4), settings->pointsA.size(), &pointsA[0], &pointsABuffer);
	BufferCreationHelper::CreateStructuredBuffer(renderer, sizeof(XMFLOAT4), settings->pointsB.size(), &pointsB[0], &pointsBBuffer);
	BufferCreationHelper::CreateStructuredBuffer(renderer, sizeof(XMFLOAT4), settings->pointsC.size(), &pointsC[0], &pointsCBuffer);

	// Create SRV to structured buffers
	BufferCreationHelper::CreateBufferSRV(renderer, pointsABuffer, &pointsABufferSRV);
	BufferCreationHelper::CreateBufferSRV(renderer, pointsBBuffer, &pointsBBufferSRV);
	BufferCreationHelper::CreateBufferSRV(renderer, pointsCBuffer, &pointsCBufferSRV);
	BufferCreationHelper::CreateConstantBuffer(renderer, sizeof(WorleyBufferType), &worleyBuffer);

	// Clean up dynamically created arrays
	delete[] pointsA;
	delete[] pointsB;
	delete[] pointsC;


	// Create a mapped resource object to map the data from the buffers to and pass them into the shader
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Fill Worley settings
	WorleyBufferType* worleyPtr;
	dc->Map(worleyBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	worleyPtr = (WorleyBufferType*)mappedResource.pData;
	worleyPtr->numCells = XMFLOAT4(settings->numCellsA, settings->numCellsB, settings->numCellsC, 0);
	worleyPtr->noisePersistence = settings->persistence;
	worleyPtr->channel = settings->channel;
	dc->Unmap(worleyBuffer, 0);


	// Set UAVs
	dc->CSSetUnorderedAccessViews(0, 1, &m_uavAccess, 0);

	// Set SRVs
	dc->CSSetShaderResources(0, 1, &pointsABufferSRV);
	dc->CSSetShaderResources(1, 1, &pointsBBufferSRV);
	dc->CSSetShaderResources(2, 1, &pointsCBufferSRV);

	// Set Constant buffers
	dc->CSSetConstantBuffers(0, 1, &worleyBuffer);
}

void WorleyNoiseShader::createGPUViews()
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

void WorleyNoiseShader::unbind(ID3D11DeviceContext* dc)
{
	ID3D11ShaderResourceView* nullSRV[] = { NULL };
	dc->CSSetShaderResources(0, 1, nullSRV);
	dc->CSSetShaderResources(1, 1, nullSRV);
	dc->CSSetShaderResources(2, 1, nullSRV);
	dc->CSSetShaderResources(3, 1, nullSRV);

	// Unbind output from compute shader
	ID3D11UnorderedAccessView* nullUAV[] = { NULL };
	dc->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

	ID3D11Buffer* nullConstBuffer[] = { NULL };
	dc->CSSetConstantBuffers(0, 1, nullConstBuffer);

	// Disable Compute Shader
	dc->CSSetShader(nullptr, nullptr, 0);
}

void WorleyNoiseShader::SetNoiseSettings(WorleyNoiseSettings val, TextureChannel channel)
{
	WorleyNoiseSettings* settings = nullptr;

	switch (channel)
	{
	case TextureChannel::RED:
		settings = &redChannelSettings;
		break;
	case TextureChannel::GREEN:
		settings = &greenChannelSettings;
		break;
	case TextureChannel::BLUE:
		settings = &blueChannelSettings;
		break;
	case TextureChannel::ALPHA:
		settings = &alphaChannelSettings;
		break;
	default:
		break;
	}

	// If settings wasn't set, return from the function
	if (settings == nullptr)
		return;

	*settings = val;
}

void WorleyNoiseShader::initShader(const wchar_t* cfile, const wchar_t* blank)
{
	// Load the shader and create the views
	loadComputeShader(cfile);
	createGPUViews();
}

std::vector<XMFLOAT4>& WorleyNoiseShader::GenerateWorleyNoisePoints(int seed, int numCells)
{
	std::vector<XMFLOAT4>* newPoints = new std::vector<XMFLOAT4>();

	// Set the seed
	srand(seed);

	// Get the cellsize by dividing the number of cells by one as textures go from 0 to 1
	float cellSize = 1.f / numCells;

	// Loop through all the cells
	for (size_t z = 0; z < numCells; z++)
	{
		for (size_t y = 0; y < numCells; y++)
		{
			for (size_t x = 0; x < numCells; x++)
			{
				// Find a random point inside the current cell and push it to the vector
				XMFLOAT3 randomFloats = XMFLOAT3((rand() / (double)RAND_MAX), (rand() / (double)RAND_MAX), (rand() / (double)RAND_MAX));
				XMFLOAT3 randomOffset = XMFLOAT3(randomFloats.x * cellSize, randomFloats.y * cellSize, randomFloats.z * cellSize);
				XMFLOAT3 cellCorner = XMFLOAT3(x * cellSize, y * cellSize, z * cellSize);
				newPoints->push_back(XMFLOAT4(cellCorner.x + randomOffset.x, cellCorner.y + randomOffset.y, cellCorner.z + randomOffset.z, 0));
			}
		}
	}

	return *newPoints;
}
