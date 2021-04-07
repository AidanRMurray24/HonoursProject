#pragma once

#ifndef CLEAN_POINTER
#define CLEAN_POINTER(val) {if (val) {delete val; val = 0;}}
#endif

struct Assets
{
	// Shaders
	class CloudMarcherShader* cloudMarcherShader = nullptr;
	class ManipulationShader* manipulationShader = nullptr;
	class SimpleRayMarcherShader* rayMarcherShader = nullptr;
	class WorleyNoiseShader* shapeNoiseGenShader = nullptr;
	class WorleyNoiseShader* detailNoiseGenShader = nullptr;
	class TextureShader* tex2DShader = nullptr;
	class TextureShader* tex3DShader = nullptr;
	class DepthShader* depthShader = nullptr;
	class CloudFragShader* cloudFragShader = nullptr;
	class PerlinNoiseShader* perlinNoiseShader = nullptr;
	class PerlinWorleyShader* perlinWorleyShader = nullptr;
	class WeatherMapShader* weatherMapShader = nullptr;

	// Meshes
	class CubeMesh* cubeMesh = nullptr;
	class PlaneMesh* planeMesh = nullptr;
	class OrthoMesh* screenOrthoMesh = nullptr;
	class OrthoMesh* noiseGenOrthoMesh = nullptr;

	// Texture
	class ID3D11ShaderResourceView* brickTexture = nullptr;
	class ID3D11ShaderResourceView* terrainColourTexture = nullptr;
	class ID3D11ShaderResourceView* terrainHeightMapTexture = nullptr;
	class ID3D11ShaderResourceView* blueNoiseTexture = nullptr;

	void CleanUp()
	{
		// Shaders
		CLEAN_POINTER(cloudMarcherShader);
		CLEAN_POINTER(manipulationShader);
		CLEAN_POINTER(rayMarcherShader);
		CLEAN_POINTER(shapeNoiseGenShader);
		CLEAN_POINTER(detailNoiseGenShader);
		CLEAN_POINTER(tex2DShader);
		CLEAN_POINTER(tex3DShader);
		CLEAN_POINTER(depthShader);
		CLEAN_POINTER(cloudFragShader);
		CLEAN_POINTER(perlinNoiseShader);
		CLEAN_POINTER(perlinWorleyShader);
		CLEAN_POINTER(weatherMapShader);

		// Meshes
		CLEAN_POINTER(cubeMesh);
		CLEAN_POINTER(planeMesh);
		CLEAN_POINTER(screenOrthoMesh);
		CLEAN_POINTER(noiseGenOrthoMesh);

		// Textures
		brickTexture = nullptr;
		terrainColourTexture = nullptr;
		terrainHeightMapTexture = nullptr;
	}
};