#include "NoiseGeneratorShader.h"
#include <stdlib.h>
#include <time.h>

NoiseGeneratorShader::NoiseGeneratorShader(ID3D11Device* _device, HWND hwnd, int w, int h, int d) : BaseShader(_device, hwnd)
{
	pointsABuffer = nullptr;
	pointsABufferSRV = nullptr;

	device = _device;
	texWidth = w;
	texHeight = h;
	texDepth = d;
	worleySettings.pointsA = GenerateWorleyNoisePoints(worleySettings.seed, worleySettings.numCellsA);
	worleySettings.pointsB = GenerateWorleyNoisePoints(worleySettings.seed, worleySettings.numCellsB);
	worleySettings.pointsC = GenerateWorleyNoisePoints(worleySettings.seed, worleySettings.numCellsC);

	initShader(L"noiseGen_cs.cso", NULL);
}

NoiseGeneratorShader::~NoiseGeneratorShader()
{
}

void NoiseGeneratorShader::setShaderParameters(ID3D11DeviceContext* dc, float tileVal)
{

	// Create a mapped resource object to map the data from the buffers to and pass them into the shader
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Fill Worley settings
	WorleyBufferType* worleyPtr;
	dc->Map(worleyBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	worleyPtr = (WorleyBufferType*)mappedResource.pData;
	worleyPtr->numCells = XMFLOAT4(worleySettings.numCellsA, worleySettings.numCellsB, worleySettings.numCellsC, 0);
	worleyPtr->noisePersistence = 0.5f;
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

void NoiseGeneratorShader::createGPUViews()
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

void NoiseGeneratorShader::unbind(ID3D11DeviceContext* dc)
{
	ID3D11ShaderResourceView* nullSRV[] = { NULL };
	dc->CSSetShaderResources(0, 1, nullSRV);

	// Unbind output from compute shader
	ID3D11UnorderedAccessView* nullUAV[] = { NULL };
	dc->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

	// Disable Compute Shader
	dc->CSSetShader(nullptr, nullptr, 0);
}

void NoiseGeneratorShader::initShader(const wchar_t* cfile, const wchar_t* blank)
{
	// Load the shader and create the views
	loadComputeShader(cfile);
	createGPUViews();


	// Fill in point data
	XMFLOAT4* pointsA = new XMFLOAT4[worleySettings.pointsA.size()];	// REMEMBER TO DELETE THIS IN DECONSTRUCTOR
	for (size_t i = 0; i < worleySettings.pointsA.size(); i++)
		pointsA[i] = worleySettings.pointsA[i];

	// Fill in point data
	XMFLOAT4* pointsB = new XMFLOAT4[worleySettings.pointsB.size()];	// REMEMBER TO DELETE THIS IN DECONSTRUCTOR
	for (size_t i = 0; i < worleySettings.pointsB.size(); i++)
		pointsB[i] = worleySettings.pointsB[i];

	// Fill in point data
	XMFLOAT4* pointsC = new XMFLOAT4[worleySettings.pointsC.size()];	// REMEMBER TO DELETE THIS IN DECONSTRUCTOR
	for (size_t i = 0; i < worleySettings.pointsC.size(); i++)
		pointsC[i] = worleySettings.pointsC[i];

	// Create structured buffers
	CreateStructuredBuffer(renderer, sizeof(XMFLOAT4), worleySettings.pointsA.size(), &pointsA[0], &pointsABuffer);
	CreateStructuredBuffer(renderer, sizeof(XMFLOAT4), worleySettings.pointsB.size(), &pointsB[0], &pointsBBuffer);
	CreateStructuredBuffer(renderer, sizeof(XMFLOAT4), worleySettings.pointsC.size(), &pointsC[0], &pointsCBuffer);

	// Create SRV to structured buffers
	CreateBufferSRV(renderer, pointsABuffer, &pointsABufferSRV);
	CreateBufferSRV(renderer, pointsBBuffer, &pointsBBufferSRV);
	CreateBufferSRV(renderer, pointsCBuffer, &pointsCBufferSRV);

	// Create constant buffers
	CreateConstantBuffer(renderer, sizeof(WorleyBufferType), &worleyBuffer);

	// Clean up dynamically created arrays
	delete[] pointsA;
	delete[] pointsB;
	delete[] pointsC;
}

std::vector<XMFLOAT4>& NoiseGeneratorShader::GenerateWorleyNoisePoints(int seed, int numCells)
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

HRESULT NoiseGeneratorShader::CreateStructuredBuffer(ID3D11Device* pDevice, UINT uElementSize, UINT uCount, void* pInitData, ID3D11Buffer** ppBufOut)
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

HRESULT NoiseGeneratorShader::CreateBufferSRV(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11ShaderResourceView** ppSRVOut)
{
	D3D11_BUFFER_DESC descBuf = {};
	pBuffer->GetDesc(&descBuf);

	D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
	desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
	desc.BufferEx.FirstElement = 0;

	if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
	{
		// This is a Raw Buffer

		desc.Format = DXGI_FORMAT_R32_TYPELESS;
		desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
		desc.BufferEx.NumElements = descBuf.ByteWidth / 4;
	}
	else if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
	{
		// This is a Structured Buffer

		desc.Format = DXGI_FORMAT_UNKNOWN;
		desc.BufferEx.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
	}
	else
	{
		return E_INVALIDARG;
	}

	return pDevice->CreateShaderResourceView(pBuffer, &desc, ppSRVOut);
}

void NoiseGeneratorShader::CreateConstantBuffer(ID3D11Device* renderer, UINT uElementSize, ID3D11Buffer** ppBufOut)
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
