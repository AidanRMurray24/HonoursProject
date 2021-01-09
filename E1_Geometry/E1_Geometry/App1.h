// Application.h
#ifndef _APP1_H
#define _APP1_H

// Includes
#include "../DXFramework/DXF.h"
#include "ManipulationShader.h"
#include "SimpleRayMarcherShader.h"
#include "TextureShader.h"
#include "NoiseGeneratorShader.h"
#include "GPUTimer.h"

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
	void RayMarchPass();
	void FinalPass();
	void gui();

private:
	// Shaders
	ManipulationShader* manipulationShader;
	SimpleRayMarcherShader* rayMarcherShader;
	NoiseGeneratorShader* noiseGenShader;
	TextureShader* tex2DShader;
	TextureShader* tex3DShader;

	// Render textures
	RenderTexture* rayMarchRT;
	RenderTexture* noiseGenRT;
	float noiseGenTexRes;

	// Meshes
	PlaneMesh* planeMesh;
	OrthoMesh* screenOrthoMesh;
	OrthoMesh* noiseGenOrthoMesh;
	
	// Lights
	Light* light;

	// Timers
	GPUTimer* noiseTimer;
	float elapsedTime;
	double timetaken = 9;

	bool textureGenerated;
	bool showWorleyNoiseTexture;
	float tileVal;
	float sliceVal;
};

#endif