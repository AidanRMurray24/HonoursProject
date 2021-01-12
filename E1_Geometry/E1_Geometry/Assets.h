#pragma once

struct Assets
{
	// Shaders
	class CloudMarcherShader* cloudMarcherShader = nullptr;
	class ManipulationShader* manipulationShader = nullptr;
	class SimpleRayMarcherShader* rayMarcherShader = nullptr;
	class NoiseGeneratorShader* noiseGenShader = nullptr;
	class TextureShader* tex2DShader = nullptr;
	class TextureShader* tex3DShader = nullptr;

	// Meshes
	class CubeMesh* cubeMesh = nullptr;
	class PlaneMesh* planeMesh = nullptr;
	class OrthoMesh* screenOrthoMesh = nullptr;
	class OrthoMesh* noiseGenOrthoMesh = nullptr;

	void CleanUp()
	{
		// Shaders
		if (cloudMarcherShader)
		{
			delete cloudMarcherShader;
			cloudMarcherShader = 0;
		}

		// Meshes
		if (cubeMesh)
		{
			delete cubeMesh;
			cubeMesh = 0;
		}
	}
};