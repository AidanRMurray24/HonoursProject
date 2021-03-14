// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"
#include "SystemParams.h"

// Shaders
#include "ManipulationShader.h"
#include "SimpleRayMarcherShader.h"
#include "TextureShader.h"
#include "WorleyNoiseShader.h"
#include "CloudMarcherShader.h"
#include "DepthShader.h"
#include "CloudFragShader.h"
#include "PerlinNoiseShader.h"

App1::App1()
{
	assets = nullptr;

	// Render textures
	sceneRT = nullptr;
	sceneDepthRT = nullptr;
	cloudFragRT = nullptr;

	// Lights
	light = nullptr;

	// Timers
	noiseTimer = nullptr;
	elapsedTime = 0;
	timetaken = 9;

	// Noise data
	shapeNoiseGenTexRes = 128;
	detailNoiseGenTexRes = 128;
	textureGenerated = false;
	showShapeNoiseTexture = true;
	showDetailNoiseTexture = false;
	showPerlinNoiseTexture = false;
	tileVal = 1.0f;
	sliceVal = 0;

	// Cloud Settings
	densityThreshold = 0.6f;
	densityMultiplier = 1.0f;
	densitySteps = 100;
	shapeNoiseTexOffsetArray[0] = 0;
	shapeNoiseTexOffsetArray[1] = 0;
	shapeNoiseTexOffsetArray[2] = 0;
	shapeNoiseTexScale = 40.0f;
	detailNoiseTexOffsetArray[0] = 0;
	detailNoiseTexOffsetArray[1] = 0;
	detailNoiseTexOffsetArray[2] = 0;
	detailNoiseTexScale = 40.0f;

	// Absorption settings
	lightAbsTowardsSun = 0.75f;
	lightAbsThroughCloud = 1.21f;
	darknessThreshold = 0.15f;
	lightSteps = 8;

	// Light settings
	lightColour[0] = 1.0f;
	lightColour[1] = 0.9f;
	lightColour[2] = 0.8f;

	showTerrain = false;
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int _screenWidth, int _screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	screenWidth = _screenWidth;
	screenHeight = _screenHeight;

	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);
	SystemParams::GetInstance().SetRenderer(renderer);
	SystemParams::GetInstance().SetMainCamera(camera);
	assets = &SystemParams::GetInstance().GetAssets();

	// Initialise Lights
	light = new Light;
	light->setPosition(0, 0, 0);
	light->setDiffuseColour(1.0f, 0.9f, 0.8f, 1.0f);
	light->setDirection(0.7f, -0.7f, 0.0f);

	LoadAssets(hwnd);

	// Set shape noise settings
	{
		// RED
		shapeNoiseSettings[0].seed = 0;
		shapeNoiseSettings[0].numCellsA = 5;
		shapeNoiseSettings[0].numCellsB = 10;
		shapeNoiseSettings[0].numCellsC = 15;
		shapeNoiseSettings[0].persistence = 0.5f;
		shapeNoiseSettings[0].channel = XMFLOAT4(1, 0, 0, 0);
		assets->shapeNoiseGenShader->SetNoiseSettings(shapeNoiseSettings[0], WorleyNoiseShader::TextureChannel::RED);

		// GREEN
		shapeNoiseSettings[1].seed = 0;
		shapeNoiseSettings[1].numCellsA = 5;
		shapeNoiseSettings[1].numCellsB = 10;
		shapeNoiseSettings[1].numCellsC = 15;
		shapeNoiseSettings[1].persistence = 0.5f;
		shapeNoiseSettings[1].channel = XMFLOAT4(0, 1, 0, 0);
		assets->shapeNoiseGenShader->SetNoiseSettings(shapeNoiseSettings[1], WorleyNoiseShader::TextureChannel::GREEN);

		// BLUE
		shapeNoiseSettings[2].seed = 0;
		shapeNoiseSettings[2].numCellsA = 10;
		shapeNoiseSettings[2].numCellsB = 15;
		shapeNoiseSettings[2].numCellsC = 20;
		shapeNoiseSettings[2].persistence = 0.5f;
		shapeNoiseSettings[2].channel = XMFLOAT4(0, 0, 1, 0);
		assets->shapeNoiseGenShader->SetNoiseSettings(shapeNoiseSettings[2], WorleyNoiseShader::TextureChannel::BLUE);

		// ALPHA
		shapeNoiseSettings[3].seed = 0;
		shapeNoiseSettings[3].numCellsA = 5;
		shapeNoiseSettings[3].numCellsB = 10;
		shapeNoiseSettings[3].numCellsC = 15;
		shapeNoiseSettings[3].persistence = 0.5f;
		shapeNoiseSettings[3].channel = XMFLOAT4(0, 0, 0, 1);
		assets->shapeNoiseGenShader->SetNoiseSettings(shapeNoiseSettings[3], WorleyNoiseShader::TextureChannel::ALPHA);

		noiseGenerated[0] = false;
		noiseGenerated[1] = false;
		noiseGenerated[2] = false;
		noiseGenerated[3] = false;
	}

	// Set detail noise settings
	detailNoiseSettings.seed = 0;
	detailNoiseSettings.numCellsA = 7;
	detailNoiseSettings.numCellsB = 17;
	detailNoiseSettings.numCellsC = 24;
	detailNoiseSettings.persistence = 0.5f;
	detailNoiseSettings.channel = XMFLOAT4(1, 0, 0, 0);
	assets->detailNoiseGenShader->SetNoiseSettings(detailNoiseSettings, WorleyNoiseShader::TextureChannel::RED);

	// Initialise timers
	noiseTimer = new GPUTimer(renderer->getDevice(), renderer->getDeviceContext());

	// Initialise Render Textures
	sceneRT = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, 0.1f, 1000.f);
	sceneDepthRT = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, 0.1f, 1000.f);
	cloudFragRT = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, 0.1f, 1000.f);

	// Initialise scene objects
	cloudContainer = new CloudContainer();
	sceneObjects.push_back(cloudContainer);
	terrainPlane = new TerrainPlane();
	sceneObjects.push_back(terrainPlane);
}


App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Lights
	CLEAN_POINTER(light);

	// Render Textures
	CLEAN_POINTER(sceneRT);
	CLEAN_POINTER(sceneDepthRT);

	// Timers
	CLEAN_POINTER(noiseTimer);

	// Scene Objects
	for (SceneObject* s : sceneObjects)
		CLEAN_POINTER(s);
	sceneObjects.clear();

	// Call clean up on the system params to detroy the loaded assets
	SystemParams::GetInstance().CleanUp();
}


bool App1::frame()
{
	bool result;

	result = BaseApplication::frame();
	if (!result)
	{
		return false;
	}
	
	// Render the graphics.
	result = render();
	if (!result)
	{
		return false;
	}

	// Re-calculate compute time every 5 seconds
	elapsedTime += timer->getTime();
	/*if (elapsedTime > 1)
	{
		elapsedTime = 0;
		timetaken = noiseTimer->GetTimeTaken();
	}*/

	return true;
}

bool App1::render()
{
	// Only generate the textures at the start
	//if (!textureGenerated)
	{
		textureGenerated = true;
		GenerateNoiseTextures();
	}
	GeometryPass();
	DepthPass();
	//RayMarchPass();
	CloudMarchPass();
	//CloudFragPass();
	FinalPass();

	return true;
}

void App1::GenerateNoiseTextures()
{
	// Generate shape noise texture
	WorleyNoiseShader* shader = SystemParams::GetInstance().GetAssets().shapeNoiseGenShader;
	if (!noiseGenerated[0] && elapsedTime > 0)
	{
		noiseGenerated[0] = true;
		shader->setShaderParameters(renderer->getDeviceContext(), WorleyNoiseShader::TextureChannel::RED);
		shader->compute(renderer->getDeviceContext(), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f));
		shader->unbind(renderer->getDeviceContext());
	}
	if (!noiseGenerated[1] && noiseGenerated[0] && elapsedTime > 1)
	{
		noiseGenerated[1] = true;
		shader->setShaderParameters(renderer->getDeviceContext(), WorleyNoiseShader::TextureChannel::GREEN);
		shader->compute(renderer->getDeviceContext(), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f));
		shader->unbind(renderer->getDeviceContext());
	}
	/*shader->setShaderParameters(renderer->getDeviceContext(), WorleyNoiseShader::TextureChannel::BLUE);
	shader->compute(renderer->getDeviceContext(), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f));
	shader->unbind(renderer->getDeviceContext());*/
	/*shader->setShaderParameters(renderer->getDeviceContext(), WorleyNoiseShader::TextureChannel::ALPHA);
	shader->compute(renderer->getDeviceContext(), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f));
	shader->unbind(renderer->getDeviceContext());*/

	// Generate detail noise texture
	shader = SystemParams::GetInstance().GetAssets().detailNoiseGenShader;
	shader->setShaderParameters(renderer->getDeviceContext(), WorleyNoiseShader::TextureChannel::RED);
	shader->compute(renderer->getDeviceContext(), ceil(detailNoiseGenTexRes / 8.0f), ceil(detailNoiseGenTexRes / 8.0f), ceil(detailNoiseGenTexRes / 8.0f));
	shader->unbind(renderer->getDeviceContext());

	// Generate perlin noise texture
	PerlinNoiseShader* perlinNoiseShader = SystemParams::GetInstance().GetAssets().perlinNoiseShader;
	perlinNoiseShader->setShaderParameters(renderer->getDeviceContext());
	perlinNoiseShader->compute(renderer->getDeviceContext(), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f));
	perlinNoiseShader->unbind(renderer->getDeviceContext());
}

void App1::GeometryPass()
{
	// Set the render target to the render texture
	sceneRT->setRenderTarget(renderer->getDeviceContext());
	sceneRT->clearRenderTarget(renderer->getDeviceContext(), 0.39f, 0.58f, 0.92f, 1.0f);
	
	// Render scene objects
	if (showTerrain)
		terrainPlane->Render(light);
	//cloudContainer->Render();

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::DepthPass()
{
	// Set the render target to the render texture
	sceneDepthRT->setRenderTarget(renderer->getDeviceContext());
	sceneDepthRT->clearRenderTarget(renderer->getDeviceContext(), 1, 0, 0, 1.0f);

	if (showTerrain)
		terrainPlane->RenderDepthFromCamera();

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::RayMarchPass()
{
	SimpleRayMarcherShader* shader = SystemParams::GetInstance().GetAssets().rayMarcherShader;

	// Raymarching pass using render texture of the rendered scene
	shader->setShaderParameters(renderer->getDeviceContext(), sceneRT->getShaderResourceView(), renderer->getProjectionMatrix());
	shader->compute(renderer->getDeviceContext(), ceil(sWidth / 8.0f), ceil(sHeight / 8.0f), ceil(sHeight / 8.0f));
	shader->unbind(renderer->getDeviceContext());
}

void App1::CloudMarchPass()
{
	CloudMarcherShader* shader = SystemParams::GetInstance().GetAssets().cloudMarcherShader;

	// Generate clouds
	shader->setShaderParameters(renderer->getDeviceContext(), sceneRT->getShaderResourceView(), sceneDepthRT->getShaderResourceView(), assets->shapeNoiseGenShader->getSRV(), assets->detailNoiseGenShader->getSRV(), renderer->getProjectionMatrix(), cloudContainer);
	shader->compute(renderer->getDeviceContext(), ceil(screenWidth / 8.0f), ceil(screenHeight / 8.0f), 1);
	shader->unbind(renderer->getDeviceContext());
}

void App1::CloudFragPass()
{
	// Set the render target to be the render to texture and clear it
	cloudFragRT->setRenderTarget(renderer->getDeviceContext());
	cloudFragRT->clearRenderTarget(renderer->getDeviceContext(), 0.0f, 0.0f, 1.0f, 1.0f);

	XMMATRIX worldMatrix = renderer->getWorldMatrix();

	renderer->setZBuffer(false);
	assets->screenOrthoMesh->sendData(renderer->getDeviceContext());
	assets->cloudFragShader->setShaderParameters(renderer->getDeviceContext(), sceneRT->getShaderResourceView(), sceneDepthRT->getShaderResourceView(), assets->shapeNoiseGenShader->getSRV(), worldMatrix, cloudFragRT->getOrthoMatrix(), cloudContainer);
	assets->cloudFragShader->render(renderer->getDeviceContext(), assets->screenOrthoMesh->getIndexCount());
	renderer->setZBuffer(true);

	// Reset the render target back to the original back buffer and not the render to texture anymore.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::FinalPass()
{
	// Clear the scene. (default blue colour)
	renderer->beginScene(0.39f, 0.58f, 0.92f, 1.0f);

	// Generate the view matrix based on the camera's position.
	camera->update();

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX orthoMatrix = renderer->getOrthoMatrix();
	XMMATRIX baseViewMatrix = camera->getOrthoViewMatrix();

	renderer->setZBuffer(false);
	
	//// Render ortho mesh with ray march
	//assets->screenOrthoMesh->sendData(renderer->getDeviceContext());
	//assets->tex2DShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, assets->rayMarcherShader->getSRV());
	//assets->tex2DShader->render(renderer->getDeviceContext(), assets->screenOrthoMesh->getIndexCount());

	//// Render ortho mesh with depth map
	//assets->screenOrthoMesh->sendData(renderer->getDeviceContext());
	//assets->tex2DShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, sceneDepthRT->getShaderResourceView());
	//assets->tex2DShader->render(renderer->getDeviceContext(), assets->screenOrthoMesh->getIndexCount());

	// Render clouds
	assets->screenOrthoMesh->sendData(renderer->getDeviceContext());
	assets->tex2DShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, assets->cloudMarcherShader->getSRV());
	assets->tex2DShader->render(renderer->getDeviceContext(), assets->screenOrthoMesh->getIndexCount());

	//// Render frag clouds
	//assets->screenOrthoMesh->sendData(renderer->getDeviceContext());
	//assets->tex2DShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, cloudFragRT->getShaderResourceView());
	//assets->tex2DShader->render(renderer->getDeviceContext(), assets->screenOrthoMesh->getIndexCount());

	// Render ortho mesh with noise texture
	if (showShapeNoiseTexture)
	{
		assets->noiseGenOrthoMesh->sendData(renderer->getDeviceContext());
		assets->tex3DShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, assets->shapeNoiseGenShader->getSRV(), sliceVal, tileVal);
		assets->tex3DShader->render(renderer->getDeviceContext(), assets->noiseGenOrthoMesh->getIndexCount());
	}
	if (showDetailNoiseTexture)
	{
		assets->noiseGenOrthoMesh->sendData(renderer->getDeviceContext());
		assets->tex3DShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, assets->detailNoiseGenShader->getSRV(), sliceVal, tileVal);
		assets->tex3DShader->render(renderer->getDeviceContext(), assets->noiseGenOrthoMesh->getIndexCount());
	}
	if (showPerlinNoiseTexture)
	{
		assets->noiseGenOrthoMesh->sendData(renderer->getDeviceContext());
		assets->tex3DShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, assets->perlinNoiseShader->getSRV(), sliceVal, tileVal);
		assets->tex3DShader->render(renderer->getDeviceContext(), assets->noiseGenOrthoMesh->getIndexCount());
	}

	renderer->setZBuffer(true);

	// Render GUI
	gui();

	// Present the rendered scene to the screen.
	renderer->endScene();
}

void App1::gui()
{
	// Force turn off unnecessary shader stages.
	renderer->getDeviceContext()->GSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->HSSetShader(NULL, NULL, 0);
	renderer->getDeviceContext()->DSSetShader(NULL, NULL, 0);

	// Build UI
	ImGui::Text("FPS: %.2f", timer->getFPS());
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);
	ImGui::Checkbox("Show Terrain", &showTerrain);

	// Noise settings
	if (ImGui::CollapsingHeader("Noise Settings"))
	{
		ImGui::Text("Noise Compute-time(ms): %.5f", timetaken * 1000);
		ImGui::Checkbox("ShowShapeNoiseTexture", &showShapeNoiseTexture);
		ImGui::Checkbox("ShowDetailNoiseTexture", &showDetailNoiseTexture);
		ImGui::Checkbox("ShowPerlinNoiseTexture", &showPerlinNoiseTexture);
		ImGui::SliderFloat("TileValue", &tileVal, 0, 10);
		ImGui::SliderFloat("Slice", &sliceVal, 0, 1);
	}

	// Cloud settings
	CloudMarcherShader* cloudShader = assets->cloudMarcherShader;
	if (ImGui::CollapsingHeader("Cloud Settings"))
	{
		if (ImGui::CollapsingHeader("Shape Noise"))
		{
			ImGui::SliderFloat3("ShapeNoiseOffset", shapeNoiseTexOffsetArray, 0, 100);
			ImGui::SliderFloat("ShapeNoiseScale", &shapeNoiseTexScale, 1, 200);
		}
		if (ImGui::CollapsingHeader("Detail Noise"))
		{
			ImGui::SliderFloat3("DetailNoiseOffset", detailNoiseTexOffsetArray, 0, 100);
			ImGui::SliderFloat("DetailNoiseScale", &detailNoiseTexScale, 1, 200);
		}
		if (ImGui::CollapsingHeader("Density Settings"))
		{
			ImGui::SliderFloat("DensityThreshold", &densityThreshold, 0, 1);
			ImGui::SliderFloat("DensityMultiplier", &densityMultiplier, 1, 10);
			ImGui::SliderInt("DensitySteps", &densitySteps, 0, 1000);
		}
	}
	cloudShader->SetShapeNoiseOffset(shapeNoiseTexOffsetArray[0], shapeNoiseTexOffsetArray[1], shapeNoiseTexOffsetArray[2]);
	cloudShader->SetShapeNoiseScale(shapeNoiseTexScale);
	cloudShader->SetDetailNoiseOffset(detailNoiseTexOffsetArray[0], detailNoiseTexOffsetArray[1], detailNoiseTexOffsetArray[2]);
	cloudShader->SetDetailNoiseScale(detailNoiseTexScale);
	cloudShader->SetDensityThreshold(densityThreshold);
	cloudShader->SetDensityMultiplier(densityMultiplier);
	cloudShader->SetDensitySteps(densitySteps);

	// Absorbtion Settings
	if (ImGui::CollapsingHeader("Absoption Settings"))
	{
		ImGui::SliderFloat("AbsTowardsSun", &lightAbsTowardsSun, 0, 5);
		ImGui::SliderFloat("AbsThroughCloud", &lightAbsThroughCloud, 0, 5);
		ImGui::SliderFloat("DarknessThreshold", &darknessThreshold, 0, 1);
		ImGui::SliderInt("LightSteps", &lightSteps, 0, 100);
	}
	cloudShader->SetLightAbsTowardsSun(lightAbsTowardsSun);
	cloudShader->SetLightAbsThroughCloud(lightAbsThroughCloud);
	cloudShader->SetDarknessThreshold(darknessThreshold);
	cloudShader->SetLightMarchSteps(lightSteps);

	// Light Settings
	if (ImGui::CollapsingHeader("Light Settings"))
	{
		ImGui::ColorPicker3("Colour", lightColour);
		light->setDiffuseColour(lightColour[0], lightColour[1], lightColour[2], 1.0f);
	}

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void App1::LoadAssets(HWND hwnd)
{
	Assets& assets = SystemParams::GetInstance().GetAssets();
	ID3D11Device* device = SystemParams::GetInstance().GetRenderer()->getDevice();
	ID3D11DeviceContext* deviceContext = SystemParams::GetInstance().GetRenderer()->getDeviceContext();

	// Initialise Shaders
	assets.manipulationShader = new ManipulationShader(device, hwnd);
	assets.rayMarcherShader = new SimpleRayMarcherShader(device, hwnd, screenWidth, screenHeight, camera, light);
	assets.tex2DShader = new TextureShader(device, hwnd, TextureType::TEXTURE2D);
	assets.tex3DShader = new TextureShader(device, hwnd, TextureType::TEXTURE3D);
	assets.shapeNoiseGenShader = new WorleyNoiseShader(device, hwnd, shapeNoiseGenTexRes, shapeNoiseGenTexRes, shapeNoiseGenTexRes);
	assets.detailNoiseGenShader = new WorleyNoiseShader(device, hwnd, shapeNoiseGenTexRes, shapeNoiseGenTexRes, shapeNoiseGenTexRes);
	assets.cloudMarcherShader = new CloudMarcherShader(device, hwnd, screenWidth, screenHeight, camera, light);
	assets.depthShader = new DepthShader(device, hwnd);
	assets.cloudFragShader = new CloudFragShader(device, hwnd, screenWidth, screenHeight, camera);
	assets.perlinNoiseShader = new PerlinNoiseShader(device, hwnd, shapeNoiseGenTexRes, shapeNoiseGenTexRes, shapeNoiseGenTexRes);

	// Initialise Meshes
	assets.cubeMesh = new CubeMesh(device, deviceContext);
	assets.planeMesh = new PlaneMesh(device, deviceContext);
	assets.screenOrthoMesh = new OrthoMesh(device, deviceContext, screenWidth, screenHeight);
	assets.noiseGenOrthoMesh = new OrthoMesh(device, deviceContext, shapeNoiseGenTexRes * 2, shapeNoiseGenTexRes * 2);

	// Initialise textures
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");
	textureMgr->loadTexture(L"TerrainColour", L"res/TerrainColour.png");
	textureMgr->loadTexture(L"TerrainHeightMap", L"res/TerrainHeightMap.png");
	assets.brickTexture = textureMgr->getTexture(L"brick");
	assets.terrainColourTexture = textureMgr->getTexture(L"TerrainColour");
	assets.terrainHeightMapTexture = textureMgr->getTexture(L"TerrainHeightMap");
}
