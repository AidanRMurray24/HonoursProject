#pragma once
#include "../DXFramework/DXF.h"
#include "TextureChannel.h"

using namespace std;
using namespace DirectX;

class WorleyNoiseShader : public BaseShader
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
		XMFLOAT4 channel = XMFLOAT4(1,0,0,0);
	};


private:
	struct WorleyBufferType
	{
		XMFLOAT4 numCells;
		XMFLOAT4 channel;
		XMFLOAT3 padding;
		float noisePersistence = .5f;
	};

public:
	WorleyNoiseShader(ID3D11Device* device, HWND hwnd, int w, int h, int d);
	~WorleyNoiseShader();

	void setShaderParameters(ID3D11DeviceContext* dc, TextureChannel channel);
	void createGPUViews();
	void unbind(ID3D11DeviceContext* dc);

	// Getters
	inline ID3D11ShaderResourceView* getSRV() { return m_srvTexOutput; }

	// Setters
	void SetNoiseSettings(WorleyNoiseSettings val, TextureChannel channel);

private:
	void initShader(const wchar_t* cfile, const wchar_t* blank);
	std::vector<XMFLOAT4>& GenerateWorleyNoisePoints(int seed, int numCells);

	// Texture set
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
	WorleyNoiseSettings redChannelSettings;
	WorleyNoiseSettings greenChannelSettings;
	WorleyNoiseSettings blueChannelSettings;
	WorleyNoiseSettings alphaChannelSettings;

	// Texture sizes
	int texWidth;
	int texHeight;
	int texDepth;

	ID3D11Device* device;
};

