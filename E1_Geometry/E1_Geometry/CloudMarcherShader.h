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

	struct CloudSettingsBufferType
	{
		XMFLOAT4 noiseTexTransform; // Offset = (x,y,z), Scale = w
		XMFLOAT4 densitySettings; // Density Threshold = x, Density Multiplier = y, Density Steps = z
	};

	struct LightBufferType
	{
		XMFLOAT4 direction;
		XMFLOAT4 position;
		XMFLOAT4 colour;
		XMFLOAT4 absorptionData; // Absorption to sun = x, Absorption through cloud = y, Darkness Threshold = z, Marching steps = w
	};

public:
	CloudMarcherShader(ID3D11Device* device, HWND hwnd, int _screenWidth, int _screenHeight, Camera* _cam, Light* _mainLight);
	~CloudMarcherShader();

	void setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* sourceTexture, ID3D11ShaderResourceView* depthMap, ID3D11ShaderResourceView* noiseTex, const XMMATRIX& projectionMatrix, CloudContainer* container);
	void createGPUViews();
	void unbind(ID3D11DeviceContext* dc);
	inline ID3D11ShaderResourceView* getSRV() { return srvTexOutput; }

	// Setters
	inline void SetDensityThreshold(float val) { cloudSettings.densitySettings.x = val; }
	inline void SetDensityMultiplier(float val) { cloudSettings.densitySettings.y = val; }
	inline void SetDensitySteps(int val) { cloudSettings.densitySettings.z = val; }
	inline void SetNoiseOffset(XMFLOAT3 val) 
	{
		cloudSettings.noiseTexTransform.x = val.x;
		cloudSettings.noiseTexTransform.y = val.y;
		cloudSettings.noiseTexTransform.z = val.z;
	}
	inline void SetNoiseOffset(float x, float y, float z) 
	{
		cloudSettings.noiseTexTransform.x = x;
		cloudSettings.noiseTexTransform.y = y;
		cloudSettings.noiseTexTransform.z = z;
	}
	inline void SetNoiseScale(float val) { cloudSettings.noiseTexTransform.w = val; }
	inline void SetLightAbsTowardsSun(float val) { absorptionData.x = val; }
	inline void SetLightAbsThroughCloud(float val) { absorptionData.y = val; }
	inline void SetDarknessThreshold(float val) { absorptionData.z = val; }
	inline void SetLightMarchSteps(int val) { absorptionData.w = val; }

private:
	void initShader(const wchar_t* cfile, const wchar_t* blank);
	void InitBuffers();
	void CreateConstantBuffer(ID3D11Device* renderer, UINT uElementSize, ID3D11Buffer** ppBufOut);

	// Views
	ID3D11ShaderResourceView* srvTexOutput;
	ID3D11UnorderedAccessView* uavTexAccess;

	// Textures
	ID3D11Texture2D* outputTexture;

	// Buffers
	ID3D11Buffer* cameraBuffer;
	ID3D11Buffer* containerInfoBuffer;
	ID3D11Buffer* cloudSettingsBuffer;
	ID3D11Buffer* lightBuffer;

	// Samplers
	ID3D11SamplerState* sampleState;

	// Data
	CloudSettingsBufferType cloudSettings;
	XMFLOAT4 absorptionData;

	// Screen dimentions
	int screenWidth;
	int screenHeight;

	// Scene Info
	Camera* cam;
	Light* mainLight;
};

