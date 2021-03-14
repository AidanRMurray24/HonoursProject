// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "../DXFramework/DXF.h"
#include "GPUTimer.h"
#include <vector>
#include "WorleyNoiseShader.h"

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
	float densityThreshold;
	float densityMultiplier;
	int densitySteps;

	// Absorption settings
	float lightAbsTowardsSun;
	float lightAbsThroughCloud;
	float darknessThreshold;
	int lightSteps;

	// Noise data
	bool textureGenerated;
	bool showShapeNoiseTexture;
	bool showDetailNoiseTexture;
	bool showPerlinNoiseTexture;
	float shapeNoiseGenTexRes;
	float detailNoiseGenTexRes;
	float tileVal;
	float sliceVal;
	bool noiseGenerated[4];
	WorleyNoiseShader::WorleyNoiseSettings shapeNoiseSettings[4];
	WorleyNoiseShader::WorleyNoiseSettings detailNoiseSettings;

	// Light Settings
	float lightColour[3];
};

#endif