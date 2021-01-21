// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "../DXFramework/DXF.h"
#include "GPUTimer.h"
#include <vector>

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
	void NoiseGenPass();
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

	bool textureGenerated;
	bool showWorleyNoiseTexture;
	float noiseGenTexRes;
	float tileVal;
	float sliceVal;
};

#endif