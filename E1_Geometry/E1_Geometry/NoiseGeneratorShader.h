#pragma once
#include "../DXFramework/DXF.h"

using namespace std;
using namespace DirectX;

class NoiseGeneratorShader : public BaseShader
{
public:
	struct WorleyNoiseSettings
	{
		int seed = 0;
		int numCellsA = 5;
		int numCellsB = 10;
		int numCellsC = 15;
		std::vector<XMFLOAT4> pointsA;
		std::vector<XMFLOAT4> pointsB;
		std::vector<XMFLOAT4> pointsC;
		float persistence = .5f;
	};

private:
	struct WorleyBufferType
	{
		XMFLOAT4 numCells;
		XMFLOAT3 padding;
		float noisePersistence = .5f;
	};

public:
	NoiseGeneratorShader(ID3D11Device* device, HWND hwnd, int w, int h, int d);
	~NoiseGeneratorShader();

	void setShaderParameters(ID3D11DeviceContext* dc, float tileVal);
	void createGPUViews();
	void unbind(ID3D11DeviceContext* dc);

	// Getters
	inline ID3D11ShaderResourceView* getSRV() { return m_srvTexOutput; }

	// Setters
	void SetNoiseSettings(WorleyNoiseSettings val);

private:
	void initShader(const wchar_t* cfile, const wchar_t* blank);
	std::vector<XMFLOAT4>& GenerateWorleyNoisePoints(int seed, int numCells);
	HRESULT CreateStructuredBuffer(ID3D11Device* pDevice, UINT uElementSize, UINT uCount, void* pInitData, ID3D11Buffer** ppBufOut);
	HRESULT CreateBufferSRV(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11ShaderResourceView** ppSRVOut);
	void CreateConstantBuffer(ID3D11Device* renderer, UINT uElementSize, ID3D11Buffer** ppBufOut);

	// Texture set
	ID3D11Texture2D* tex2D;
	ID3D11Texture3D* tex3D;
	ID3D11ShaderResourceView* m_srvTexOutput;
	ID3D11UnorderedAccessView* m_uavAccess;

	// Structured Buffers
	ID3D11Buffer* pointsABuffer;
	ID3D11Buffer* pointsBBuffer;
	ID3D11Buffer* pointsCBuffer;
	ID3D11ShaderResourceView* pointsABufferSRV;
	ID3D11ShaderResourceView* pointsBBufferSRV;
	ID3D11ShaderResourceView* pointsCBufferSRV;

	// Buffers
	ID3D11Buffer* worleyBuffer;

	// Worley settings
	WorleyNoiseSettings worleySettings;

	// Texture sizes
	int texWidth;
	int texHeight;
	int texDepth;

	ID3D11Device* device;
};

