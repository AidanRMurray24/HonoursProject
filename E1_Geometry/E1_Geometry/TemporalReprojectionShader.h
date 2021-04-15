#pragma once
#include "../DXFramework/DXF.h"

using namespace DirectX;

class TemporalReprojectionShader : public BaseShader
{
private:
	struct ReprojectionInfoBufferType
	{
		XMMATRIX oldViewProjMatrix, currentInvViewProjMatrix;
		float currentFrame;
		XMFLOAT3 padding;
	};

public:
	TemporalReprojectionShader(ID3D11Device* device, HWND hwnd, int _screenWidth, int _screenHeight);
	~TemporalReprojectionShader();

	void setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* currentFrameTex, ID3D11ShaderResourceView* previousFrameTex, XMMATRIX oldViewProjMatrix, XMMATRIX currentInvViewProjMatrix, int currentFrame);
	void createGPUViews();
	void unbind(ID3D11DeviceContext* dc);

	// Getters
	inline ID3D11ShaderResourceView* getSRV() { return srvTexOutput; }

private:
	void initShader(const wchar_t* cfile, const wchar_t* blank);
	void InitBuffers();

	// Views
	ID3D11ShaderResourceView* srvTexOutput;
	ID3D11UnorderedAccessView* uavTexAccess;

	// Textures
	ID3D11Texture2D* outputTexture;

	// Buffers
	ID3D11Buffer* infoBuffer;

	// Samplers
	ID3D11SamplerState* sampleState;

	// Screen dimentions
	int screenWidth;
	int screenHeight;
};

