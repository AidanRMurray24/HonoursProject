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
		XMFLOAT4 shapeNoiseTexTransform; // Offset = (x,y,z), Scale = w
		XMFLOAT4 detailNoiseTexTransform; // Offset = (x,y,z), Scale = w
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

	void setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* sourceTexture, ID3D11ShaderResourceView* depthMap, ID3D11ShaderResourceView* shapeNoiseTex, ID3D11ShaderResourceView* detailNoiseTex, const XMMATRIX& projectionMatrix, CloudContainer* container);
	void createGPUViews();
	void unbind(ID3D11DeviceContext* dc);
	inline ID3D11ShaderResourceView* getSRV() { return srvTexOutput; }

	// Setters
	inline void SetDensityThreshold(float val) { cloudSettings.densitySettings.x = val; }
	inline void SetDensityMultiplier(float val) { cloudSettings.densitySettings.y = val; }
	inline void SetDensitySteps(int val) { cloudSettings.densitySettings.z = val; }
	inline void SetShapeNoiseOffset(XMFLOAT3 val) 
	{
		cloudSettings.shapeNoiseTexTransform.x = val.x;
		cloudSettings.shapeNoiseTexTransform.y = val.y;
		cloudSettings.shapeNoiseTexTransform.z = val.z;
	}
	inline void SetShapeNoiseOffset(float x, float y, float z) 
	{
		cloudSettings.shapeNoiseTexTransform.x = x;
		cloudSettings.shapeNoiseTexTransform.y = y;
		cloudSettings.shapeNoiseTexTransform.z = z;
	}
	inline void SetShapeNoiseScale(float val) { cloudSettings.shapeNoiseTexTransform.w = val; }
	inline void SetDetailNoiseOffset(XMFLOAT3 val)
	{
		cloudSettings.detailNoiseTexTransform.x = val.x;
		cloudSettings.detailNoiseTexTransform.y = val.y;
		cloudSettings.detailNoiseTexTransform.z = val.z;
	}
	inline void SetDetailNoiseOffset(float x, float y, float z)
	{
		cloudSettings.detailNoiseTexTransform.x = x;
		cloudSettings.detailNoiseTexTransform.y = y;
		cloudSettings.detailNoiseTexTransform.z = z;
	}
	inline void SetDetailNoiseScale(float val) { cloudSettings.detailNoiseTexTransform.w = val; }
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

