#pragma once
#include "../DXFramework/DXF.h"
#include "CloudContainer.h"

using namespace DirectX;

class CloudFragShader : public BaseShader
{
private:
	struct CameraBufferType
	{
		XMMATRIX invViewMatrix, invProjectionMatrix;
		XMFLOAT3 cameraPos;
		float padding;
	};

	struct ContainerInfoBufferType
	{
		XMFLOAT4 boundsMin;
		XMFLOAT4 boundsMax;
	};

public:
	CloudFragShader(ID3D11Device* device, HWND hwnd, int _screenWidth, int _screenHeight, Camera* _cam);
	~CloudFragShader();

	void setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* sourceTexture, ID3D11ShaderResourceView* depthMap, ID3D11ShaderResourceView* noiseTex, const XMMATRIX& worldMatrix, const XMMATRIX& projectionMatrix, CloudContainer* container);

private:
	void initShader(const wchar_t* vsFilename, const wchar_t* psFilename);
	void InitBuffers();

	// Buffers
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* cameraBuffer;
	ID3D11Buffer* containerInfoBuffer;

	ID3D11SamplerState* sampleState;

	// Screen dimentions
	int screenWidth;
	int screenHeight;

	// Scene Info
	Camera* cam;
};

