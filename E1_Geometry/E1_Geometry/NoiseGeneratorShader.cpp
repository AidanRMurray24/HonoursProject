#include "NoiseGeneratorShader.h"
#include <stdlib.h>
#include <time.h>

NoiseGeneratorShader::NoiseGeneratorShader(ID3D11Device* _device, HWND hwnd, int w, int h, int d) : BaseShader(_device, hwnd)
{
	pointsSeed = 0;
	device = _device;
	texWidth = w;
	texHeight = h;
	texDepth = d;
	GenerateWorleyNoisePoints();

	initShader(L"noiseGen_cs.cso", NULL);
}

NoiseGeneratorShader::~NoiseGeneratorShader()
{
}

void NoiseGeneratorShader::setShaderParameters(ID3D11DeviceContext* dc, float tileVal)
{
	// Pass the source texture and the texture to be modified to the shader
	dc->CSSetUnorderedAccessViews(0, 1, &m_uavAccess, 0);

	// Create a mapped resource object to map the data from the buffers to and pass them into the shader
	D3D11_MAPPED_SUBRESOURCE mappedResource;

	// Send the information from the point buffer to the shader
	PointBufferType* pointPtr;
	dc->Map(pointBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	pointPtr = (PointBufferType*)mappedResource.pData;
	for (size_t i = 0; i < TOTAL_CELLS; i++)
		pointPtr->points[i] = XMFLOAT4(points[i].x, points[i].y, points[i].z, 0);
	pointPtr->cellInfo.x = NUM_CELLS;
	pointPtr->cellInfo.y = TOTAL_CELLS;
	pointPtr->cellInfo.z = 1.f / NUM_CELLS;
	pointPtr->cellInfo.w = tileVal;
	dc->Unmap(pointBuffer, 0);
	dc->CSSetConstantBuffers(0, 1, &pointBuffer);
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

	// Init point buffer
	D3D11_BUFFER_DESC pointBufferDesc;
	pointBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	pointBufferDesc.ByteWidth = sizeof(PointBufferType);
	pointBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	pointBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	pointBufferDesc.MiscFlags = 0;
	pointBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&pointBufferDesc, NULL, &pointBuffer);
}

void NoiseGeneratorShader::GenerateWorleyNoisePoints()
{	
	// Set a random seed
	srand(pointsSeed);

	// Get the cellsize by dividing the number of cells by one as textures go from 0 to 1
	float cellSize = 1.f / NUM_CELLS;

	// Loop through all the cells
	for (size_t z = 0; z < NUM_CELLS; z++)
	{
		for (size_t y = 0; y < NUM_CELLS; y++)
		{
			for (size_t x = 0; x < NUM_CELLS; x++)
			{
				// Find a random point inside the current cell and push it to the vector
				XMFLOAT3 randomFloats = XMFLOAT3((rand() / (double)RAND_MAX), (rand() / (double)RAND_MAX), (rand() / (double)RAND_MAX));
				XMFLOAT3 randomOffset = XMFLOAT3(randomFloats.x * cellSize, randomFloats.y * cellSize, randomFloats.z * cellSize);
				XMFLOAT3 cellCorner = XMFLOAT3(x * cellSize, y * cellSize, z * cellSize);
				points.push_back(XMFLOAT3(cellCorner.x + randomOffset.x, cellCorner.y + randomOffset.y, cellCorner.z + randomOffset.z));
			}
		}
	}
}
