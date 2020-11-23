#include "NoiseGeneratorShader.h"
#include <stdlib.h>
#include <time.h>

NoiseGeneratorShader::NoiseGeneratorShader(ID3D11Device* device, HWND hwnd, int w, int h) : BaseShader(device, hwnd)
{
	sWidth = w;
	sHeight = h;
	GenerateWorleyNoisePoints();

	initShader(L"noiseGen_cs.cso", NULL);
}

NoiseGeneratorShader::~NoiseGeneratorShader()
{
}

void NoiseGeneratorShader::setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* texture1)
{
	dc->CSSetShaderResources(0, 1, &texture1);
	dc->CSSetUnorderedAccessViews(0, 1, &m_uavAccess, 0);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	PointBufferType* pointPtr;

	dc->Map(pointBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	pointPtr = (PointBufferType*)mappedResource.pData;
	for (size_t i = 0; i < TOTAL_CELLS; i++)
		pointPtr->points[i] = XMFLOAT4(points[i].x, points[i].y, 0, 0);
	dc->Unmap(pointBuffer, 0);
	dc->CSSetConstantBuffers(0, 1, &pointBuffer);
}

void NoiseGeneratorShader::createOutputUAV()
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
	descUAV.Format = DXGI_FORMAT_R32G32B32A32_FLOAT; // DXGI_FORMAT_UNKNOWN;
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
	loadComputeShader(cfile);
	createOutputUAV();

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
	srand(time(NULL));

	float cellSize = 1.f / NUM_CELLS;

	for (size_t x = 0; x < NUM_CELLS; x++)
	{
		for (size_t y = 0; y < NUM_CELLS; y++)
		{
			XMFLOAT2 randomFloats = XMFLOAT2((rand() / (double)RAND_MAX), (rand() / (double)RAND_MAX));
			XMFLOAT2 randomOffset = XMFLOAT2(randomFloats.x * cellSize, randomFloats.y * cellSize);
			XMFLOAT2 cellCorner = XMFLOAT2(x * cellSize, y * cellSize);

			int index = x + NUM_CELLS * (y * NUM_CELLS);
			points.push_back(XMFLOAT2(cellCorner.x + randomOffset.x, cellCorner.y + randomOffset.y));
		}
	}
}
