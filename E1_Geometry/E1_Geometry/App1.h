// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "../DXFramework/DXF.h"
#include "GPUTimer.h"
#include <vector>
#include "WorleyNoiseShader.h"
#include "CloudMarcherShader.h"
#include "PerformanceTest.h"
#include "CoverageTest.h"
#include "DistanceTest.h"
#include "StepSizeTest.h"
#include "LightStepsTest.h"
#include "ReprojectionTest.h"

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
	void CloudMarchPass();
	void ReprojectionPass();
	void FinalPass();
	void gui();

private:
	void LoadAssets(HWND hwnd);
	void DrawMessageBox(std::string windowTag, std::string message, ImVec4 messageColour);
	void StartTest(PerformanceTest* test);
	void CancelTest();

	class Assets* assets;

	// Render textures
	RenderTexture* sceneRT;
	RenderTexture* cloudFragRT;
	
	// Lights
	Light* light;

	// Timers
	GPUTimer* cloudMarcherShaderTimer;

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
	int reprojectionFrameCounter;
	XMMATRIX oldViewProjMatrix;
	XMMATRIX currentInvViewProjMatrix;
	bool useTemporalReprojection;
	XMFLOAT2 cloudTextureRes;

	// Absorption settings
	float lightAbsTowardsSun;
	float lightAbsThroughCloud;
	float cloudBrightness;
	int lightSteps;

	// Scatter settings
	CloudMarcherShader::ScatterSettings scatterSettings;

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
	float noiseOrthoMeshRes;
	float shapeWeights[4];
	float detailWeights[3];

	// Weather Map settings
	bool showWeatherMap;
	int weatherMapTexRes;
	CloudMarcherShader::WeatherMapTextureSettings coverageTexSettings;
	CloudMarcherShader::WeatherMapTextureSettings heightTexSettings;
	float weatherChannelIntensities[2];

	// Light Settings
	float lightColour[3];

	// Testing
	float elapsedTime;
	double timetaken;
	bool recordTimeTaken;
	bool isTimeRecorded;
	bool testStarted;
	bool testFinished;
	float timeBetweenRecordings;
	int numRecordingToAverage;
	std::vector<float> computeTimes;
	PerformanceTest* currentTest;
	CoverageTest* coverageTest;
	DistanceTest* distanceTest;
	StepSizeTest* stepSizeTest;
	LightStepsTest* lightStepsTest;
	ReprojectionTest* reprojectionTest;
	float estimatedTimeRemaining;
};

#endif