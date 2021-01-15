#pragma once
#include "../DXFramework/DXF.h"
#include "CloudContainer.h"

using namespace DirectX;

class CloudMarcherShader : public BaseShader
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
	CloudMarcherShader(ID3D11Device* device, HWND hwnd, int _screenWidth, int _screenHeight, Camera* _cam);
	~CloudMarcherShader();

	void setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* sourceTexture, ID3D11ShaderResourceView* depthMap, const XMMATRIX& projectionMatrix, CloudContainer* container);
	void createGPUViews();
	void unbind(ID3D11DeviceContext* dc);
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
	ID3D11Buffer* cameraBuffer;
	ID3D11Buffer* containerInfoBuffer;

	ID3D11SamplerState* sampleState;

	// Screen dimentions
	int screenWidth;
	int screenHeight;

	// Scene Info
	Camera* cam;
};

