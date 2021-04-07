// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "../DXFramework/DXF.h"
#include "GPUTimer.h"
#include <vector>
#include "WorleyNoiseShader.h"
#include "CloudMarcherShader.h"

// Scene objects
#include "CloudContainer.h"
#include "TerrainPlane.h"

class App1 : public BaseApplication
{
public:

	App1();
	~App1();
	void init(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight, Input* in, bool VSYNC, bool FULL_SCREEN);

	bool frame();

protected:
	bool render();
	void GenerateWeatherMap();
	void GenerateNoiseTextures();
	void GeometryPass();
	void DepthPass();
	void RayMarchPass();
	void CloudMarchPass();
	void CloudFragPass();
	void FinalPass();
	void gui();

private:
	void LoadAssets(HWND hwnd);

	class Assets* assets;

	// Render textures
	RenderTexture* sceneRT;
	RenderTexture* sceneDepthRT;
	RenderTexture* cloudFragRT;
	
	// Lights
	Light* light;

	// Timers
	GPUTimer* noiseTimer;
	float elapsedTime;
	double timetaken = 9;

	// Scene objects
	std::vector<SceneObject*> sceneObjects;
	CloudContainer* cloudContainer;
	TerrainPlane* terrainPlane;

	// Screen info
	int screenWidth;
	int screenHeight;
	bool showTerrain;

	// Cloud Settings
	float shapeNoiseTexOffsetArray[3];
	float shapeNoiseTexScale;
	float detailNoiseTexOffsetArray[3];
	float detailNoiseTexScale;
	float globalCoverage;
	float densityMultiplier;
	int densitySteps;
	float stepSize;
	float edgeFadePercent;

	// Absorption settings
	float lightAbsTowardsSun;
	float lightAbsThroughCloud;
	float cloudBrightness;
	int lightSteps;

	// Noise data
	bool textureGenerated;
	bool showShapeNoiseTexture;
	bool showDetailNoiseTexture;
	bool showPerlinNoiseTexture;
	bool usePerlinNoise;
	int shapeNoiseGenTexRes;
	int detailNoiseGenTexRes;
	float tileVal;
	float sliceVal;
	WorleyNoiseShader::WorleyNoiseSettings shapeNoiseSettings;
	WorleyNoiseShader::WorleyNoiseSettings detailNoiseSettings;
	float blueNoiseOffsetStrength;

	// Weather Map settings
	bool showWeatherMap;
	int weatherMapTexRes;
	CloudMarcherShader::WeatherMapTextureSettings coverageTexSettings;

	// Light Settings
	float lightColour[3];
};

#endif