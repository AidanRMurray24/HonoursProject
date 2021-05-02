#pragma once
#include "../DXFramework/DXF.h"
#include "CloudContainer.h"
#include "TextureChannel.h"

using namespace DirectX;

class CloudMarcherShader : public BaseShader
{
public:
	struct WeatherMapTextureSettings
	{
		XMFLOAT3 offset = XMFLOAT3(0, 0, 0);
		float scale = 800;
		float intensity = 0.5f;
	};

	struct ScatterSettings
	{
		float inScatter = 0.857f;
		float outScatter = 0.278f;
		float silverLiningIntensity = 4.5f;
		float silverLiningExponent = 20;
		float inOutScatterBlend = .88f;
	};

private:
	struct CameraBufferType
	{
		XMMATRIX invViewMatrix, invProjectionMatrix, oldViewProjMatrix;
		XMFLOAT3 cameraPos;
		float padding;
	};

	struct ContainerInfoBufferType
	{
		XMFLOAT4 boundsMin;
		XMFLOAT4 boundsMax;
		float edgeFadePercentage;
		XMFLOAT3 padding;
	};

	struct CloudSettingsBufferType
	{
		XMFLOAT4 shapeNoiseTexTransform; // Offset = (x,y,z), Scale = w
		XMFLOAT4 shapeNoiseWeights;
		XMFLOAT4 detailNoiseTexTransform; // Offset = (x,y,z), Scale = w
		XMFLOAT4 detailNoiseWeights;
		XMFLOAT4 densitySettings; // Global Coverage = x, Density Multiplier = y, Density Steps = z, Step Size = w
		XMFLOAT4 optimisationSettings; // Blue noise strength = x, reprojectionFrame = y, useTemporalReprojection = z
	};

	struct LightBufferType
	{
		XMFLOAT4 direction;
		XMFLOAT4 position;
		XMFLOAT4 colour;
		XMFLOAT4 absorptionData; // Absorption to sun = x, Absorption through cloud = y, Cloud Brightness = z, Marching steps = w
		XMFLOAT4 inOutScatterSettings; // inScatter = x, outScatter = y, inOutScatterBlend = z
		XMFLOAT4 SilverLiningEffect; // silver lining intensity = x, silver lining exponent = y
	};

	struct WeatherMapBufferType
	{
		XMFLOAT4 coverageTexTransform; // Offset = (x,y,z), Scale = w
		XMFLOAT4 heightTexTransform; // Offset = (x,y,z), Scale = w
		XMFLOAT4 weatherMapIntensities; // Channel intensities
	};

public:
	CloudMarcherShader(ID3D11Device* device, HWND hwnd, int _screenWidth, int _screenHeight, Camera* _cam, Light* _mainLight);
	~CloudMarcherShader();

	void setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* sourceTexture, ID3D11ShaderResourceView* shapeNoiseTex, ID3D11ShaderResourceView* detailNoiseTex, ID3D11ShaderResourceView* weatherMap, ID3D11ShaderResourceView* blueNoise, const XMMATRIX& projectionMatrix, CloudContainer* container);
	void createGPUViews();
	void unbind(ID3D11DeviceContext* dc);

	void SaveLastFrame(ID3D11DeviceContext* dc);

	// Getters
	inline ID3D11ShaderResourceView* getSRV() { return srvTexOutput; }
	inline ID3D11ShaderResourceView* getPreviousTex() { return previousFrame; }

	// Setters
	inline void SetDensityThreshold(float val) { cloudSettings.densitySettings.x = val; }
	inline void SetDensityMultiplier(float val) { cloudSettings.densitySettings.y = val; }
	inline void SetDensitySteps(int val) { cloudSettings.densitySettings.z = val; }
	inline void SetStepSize(float val) { cloudSettings.densitySettings.w = val; }
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
	inline void SetCloudBrightness(float val) { absorptionData.z = val; }
	inline void SetLightMarchSteps(int val) { absorptionData.w = val; }
	inline void SetShapeNoiseWeights(XMFLOAT4 val) { cloudSettings.shapeNoiseWeights = val; }
	inline void SetDetailNoiseWeights(XMFLOAT4 val) { cloudSettings.detailNoiseWeights = val; }
	inline void SetEdgeFadePercentage(float val) { edgeFadePercent = val; }
	void SetWeatherMapTexSettings(WeatherMapTextureSettings settings, TextureChannel channel);
	inline void SetBlueNoiseStrength(float val) { cloudSettings.optimisationSettings.x = val; }
	inline void SetReprojectionFrame(int val) { cloudSettings.optimisationSettings.y = val; }
	inline void SetTemporalReprojection(bool val) { cloudSettings.optimisationSettings.z = val; }
	inline void SetScatterSettings(ScatterSettings val) { scatterSettings = val; }

private:
	void initShader(const wchar_t* cfile, const wchar_t* blank);
	void InitBuffers();

	// Views
	ID3D11ShaderResourceView* srvTexOutput;
	ID3D11ShaderResourceView* previousFrame;
	ID3D11UnorderedAccessView* uavTexAccess;

	// Textures
	ID3D11Texture2D* outputTexture;
	ID3D11Texture2D* previousFrameTex;

	// Buffers
	ID3D11Buffer* cameraBuffer;
	ID3D11Buffer* containerInfoBuffer;
	ID3D11Buffer* cloudSettingsBuffer;
	ID3D11Buffer* lightBuffer;
	ID3D11Buffer* weatherBuffer;

	// Samplers
	ID3D11SamplerState* sampleState;

	// Data
	CloudSettingsBufferType cloudSettings;
	XMFLOAT4 absorptionData;
	float edgeFadePercent;

	// Screen dimentions
	int screenWidth;
	int screenHeight;

	// Scene Info
	Camera* cam;
	Light* mainLight;
	XMMATRIX oldViewProjMatrix;

	// Weather Map Settings
	WeatherMapTextureSettings weatherRedChannel;
	WeatherMapTextureSettings weatherGreenChannel;

	// Scatter settings
	ScatterSettings scatterSettings;
};

