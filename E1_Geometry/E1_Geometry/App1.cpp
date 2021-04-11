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
#include "PerlinWorleyShader.h"
#include "WeatherMapShader.h"

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
	elapsedTime = 0;
	timetaken = 9;
	cloudMarcherShaderTimer = nullptr;
	recordTimeTaken = false;
	isTimeRecorded = false;

	// Noise data
	shapeNoiseGenTexRes = 128;
	detailNoiseGenTexRes = 128;
	textureGenerated = false;
	showShapeNoiseTexture = false;
	showDetailNoiseTexture = false;
	showPerlinNoiseTexture = false;
	usePerlinNoise = true;
	tileVal = 1.0f;
	sliceVal = 0;
	blueNoiseOffsetStrength = 6.f;

	// Cloud Settings
	globalCoverage = 0.7f;
	densityMultiplier = 1.0f;
	densitySteps = 1000;
	stepSize = 2.5;
	shapeNoiseTexOffsetArray[0] = 0;
	shapeNoiseTexOffsetArray[1] = 0;
	shapeNoiseTexOffsetArray[2] = 0;
	shapeNoiseTexScale = 120.0f;
	detailNoiseTexOffsetArray[0] = 0;
	detailNoiseTexOffsetArray[1] = 0;
	detailNoiseTexOffsetArray[2] = 0;
	detailNoiseTexScale = 30.0f;
	edgeFadePercent = 0.3f;
	reprojectionFrameCounter = 1;

	// Absorption settings
	lightAbsTowardsSun = 0.84f;
	lightAbsThroughCloud = 0.84f;
	cloudBrightness = 0.15f;
	lightSteps = 4;

	// Light settings
	lightColour[0] = 1;
	lightColour[1] = 1;
	lightColour[2] = 1;

	// Weather map settings
	showWeatherMap = false;
	weatherMapTexRes = 128;

	// Terrain settings
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
	//light->setDiffuseColour(1.0f, 0.9f, 0.8f, 1.0f);
	light->setDiffuseColour(1.0f, 1, 1, 1.0f);
	light->setDirection(0.7f, -0.7f, 0.0f);

	LoadAssets(hwnd);

	// Set shape noise settings
	{
		// RED
		shapeNoiseSettings.seed = 34;
		shapeNoiseSettings.numCellsA = 5;
		shapeNoiseSettings.numCellsB = 10;
		shapeNoiseSettings.numCellsC = 15;
		shapeNoiseSettings.persistence = 0.5f;
		shapeNoiseSettings.channel = XMFLOAT4(1, 0, 0, 0);
		assets->shapeNoiseGenShader->SetNoiseSettings(shapeNoiseSettings, TextureChannel::RED);

		// GREEN
		shapeNoiseSettings.seed = 23;
		shapeNoiseSettings.numCellsA = 7;
		shapeNoiseSettings.numCellsB = 12;
		shapeNoiseSettings.numCellsC = 17;
		shapeNoiseSettings.persistence = 0.5f;
		shapeNoiseSettings.channel = XMFLOAT4(0, 1, 0, 0);
		assets->shapeNoiseGenShader->SetNoiseSettings(shapeNoiseSettings, TextureChannel::GREEN);

		// BLUE
		shapeNoiseSettings.seed = 456;
		shapeNoiseSettings.numCellsA = 10;
		shapeNoiseSettings.numCellsB = 15;
		shapeNoiseSettings.numCellsC = 20;
		shapeNoiseSettings.persistence = 0.5f;
		shapeNoiseSettings.channel = XMFLOAT4(0, 0, 1, 0);
		assets->shapeNoiseGenShader->SetNoiseSettings(shapeNoiseSettings, TextureChannel::BLUE);

		// ALPHA
		shapeNoiseSettings.seed = 656;
		shapeNoiseSettings.numCellsA = 20;
		shapeNoiseSettings.numCellsB = 40;
		shapeNoiseSettings.numCellsC = 60;
		shapeNoiseSettings.persistence = 0.5f;
		shapeNoiseSettings.channel = XMFLOAT4(0, 0, 0, 1);
		assets->shapeNoiseGenShader->SetNoiseSettings(shapeNoiseSettings, TextureChannel::ALPHA);
	}

	// Set detail noise settings
	{
		// RED
		detailNoiseSettings.seed = 1;
		detailNoiseSettings.numCellsA = 5;
		detailNoiseSettings.numCellsB = 10;
		detailNoiseSettings.numCellsC = 15;
		detailNoiseSettings.persistence = 0.5f;
		detailNoiseSettings.channel = XMFLOAT4(1, 0, 0, 0);
		assets->detailNoiseGenShader->SetNoiseSettings(detailNoiseSettings, TextureChannel::RED);

		// GREEN
		detailNoiseSettings.seed = 12;
		detailNoiseSettings.numCellsA = 10;
		detailNoiseSettings.numCellsB = 15;
		detailNoiseSettings.numCellsC = 20;
		detailNoiseSettings.persistence = 0.5f;
		detailNoiseSettings.channel = XMFLOAT4(0, 1, 0, 0);
		assets->detailNoiseGenShader->SetNoiseSettings(detailNoiseSettings, TextureChannel::GREEN);

		// BLUE
		detailNoiseSettings.seed = 123;
		detailNoiseSettings.numCellsA = 20;
		detailNoiseSettings.numCellsB = 40;
		detailNoiseSettings.numCellsC = 60;
		detailNoiseSettings.persistence = 0.5f;
		detailNoiseSettings.channel = XMFLOAT4(0, 0, 1, 0);
		assets->detailNoiseGenShader->SetNoiseSettings(detailNoiseSettings, TextureChannel::BLUE);
	}

	// Set noise weights for the cloud marcher shader
	assets->cloudMarcherShader->SetShapeNoiseWeights(XMFLOAT4(1, 0.625f, 0.25f, 0.125f));
	assets->cloudMarcherShader->SetDetailNoiseWeights(XMFLOAT4(0.625f, 0.25f, 0.125f, 0.f));

	// Initialise timers
	cloudMarcherShaderTimer = new GPUTimer(renderer->getDevice(), renderer->getDeviceContext());

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
	CLEAN_POINTER(cloudMarcherShaderTimer);

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

	// Record the compute time of the cloud marcher shader
	if (isTimeRecorded)
	{
		elapsedTime += timer->getTime();
		if (elapsedTime > 0.5f)
		{
			elapsedTime = 0;
			isTimeRecorded = false;
			timetaken = cloudMarcherShaderTimer->GetTimeTaken();
		}
	}

	// Increment the reprojection frame counter and keep it between 0 and 15
	reprojectionFrameCounter += 1;
	reprojectionFrameCounter %= 16;
	assets->cloudMarcherShader->SetReprojectionFrame(reprojectionFrameCounter);

	return true;
}

bool App1::render()
{
	// Only generate the textures at the start
	if (!textureGenerated)
	{
		textureGenerated = true;
		GenerateWeatherMap();
		GenerateNoiseTextures();
	}
	GeometryPass();
	CloudMarchPass();
	FinalPass();

	return true;
}

void App1::GenerateWeatherMap()
{
	WeatherMapShader* weatherMapShader = SystemParams::GetInstance().GetAssets().weatherMapShader;
	weatherMapShader->setShaderParameters(renderer->getDeviceContext());
	weatherMapShader->compute(renderer->getDeviceContext(), ceil(weatherMapTexRes / 8.0f), ceil(weatherMapTexRes / 8.0f), ceil(weatherMapTexRes / 8.0f));
	weatherMapShader->unbind(renderer->getDeviceContext());
}

void App1::GenerateNoiseTextures()
{
	// Generate shape noise texture
	WorleyNoiseShader* worleyNoiseShader = SystemParams::GetInstance().GetAssets().shapeNoiseGenShader;
	worleyNoiseShader->setShaderParameters(renderer->getDeviceContext(), TextureChannel::RED);
	worleyNoiseShader->compute(renderer->getDeviceContext(), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f));
	worleyNoiseShader->unbind(renderer->getDeviceContext());
	worleyNoiseShader->setShaderParameters(renderer->getDeviceContext(), TextureChannel::GREEN);
	worleyNoiseShader->compute(renderer->getDeviceContext(), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f));
	worleyNoiseShader->unbind(renderer->getDeviceContext());
	worleyNoiseShader->setShaderParameters(renderer->getDeviceContext(), TextureChannel::BLUE);
	worleyNoiseShader->compute(renderer->getDeviceContext(), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f));
	worleyNoiseShader->unbind(renderer->getDeviceContext());
	worleyNoiseShader->setShaderParameters(renderer->getDeviceContext(), TextureChannel::ALPHA);
	worleyNoiseShader->compute(renderer->getDeviceContext(), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f));
	worleyNoiseShader->unbind(renderer->getDeviceContext());

	// Generate detail noise texture
	worleyNoiseShader = SystemParams::GetInstance().GetAssets().detailNoiseGenShader;
	worleyNoiseShader->setShaderParameters(renderer->getDeviceContext(), TextureChannel::RED);
	worleyNoiseShader->compute(renderer->getDeviceContext(), ceil(detailNoiseGenTexRes / 8.0f), ceil(detailNoiseGenTexRes / 8.0f), ceil(detailNoiseGenTexRes / 8.0f));
	worleyNoiseShader->unbind(renderer->getDeviceContext());
	worleyNoiseShader->setShaderParameters(renderer->getDeviceContext(), TextureChannel::GREEN);
	worleyNoiseShader->compute(renderer->getDeviceContext(), ceil(detailNoiseGenTexRes / 8.0f), ceil(detailNoiseGenTexRes / 8.0f), ceil(detailNoiseGenTexRes / 8.0f));
	worleyNoiseShader->unbind(renderer->getDeviceContext());
	worleyNoiseShader->setShaderParameters(renderer->getDeviceContext(), TextureChannel::BLUE);
	worleyNoiseShader->compute(renderer->getDeviceContext(), ceil(detailNoiseGenTexRes / 8.0f), ceil(detailNoiseGenTexRes / 8.0f), ceil(detailNoiseGenTexRes / 8.0f));
	worleyNoiseShader->unbind(renderer->getDeviceContext());

	// Generate perlin noise texture
	PerlinNoiseShader* perlinNoiseShader = SystemParams::GetInstance().GetAssets().perlinNoiseShader;
	perlinNoiseShader->setShaderParameters(renderer->getDeviceContext());
	perlinNoiseShader->compute(renderer->getDeviceContext(), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f));
	perlinNoiseShader->unbind(renderer->getDeviceContext());

	// Combine the perlin and worley textures
	PerlinWorleyShader* perlinWorleyShader = SystemParams::GetInstance().GetAssets().perlinWorleyShader;
	perlinWorleyShader->setShaderParameters(renderer->getDeviceContext(), perlinNoiseShader->getSRV(), worleyNoiseShader->getSRV());
	perlinWorleyShader->compute(renderer->getDeviceContext(), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f));
	perlinWorleyShader->unbind(renderer->getDeviceContext());
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

void App1::CloudMarchPass()
{
	CloudMarcherShader* shader = SystemParams::GetInstance().GetAssets().cloudMarcherShader;

	// Generate clouds
	if (usePerlinNoise)
	{
		shader->setShaderParameters(renderer->getDeviceContext(), sceneRT->getShaderResourceView(), sceneDepthRT->getShaderResourceView(), assets->perlinWorleyShader->getSRV(), assets->detailNoiseGenShader->getSRV(), assets->weatherMapShader->getSRV(), assets->blueNoiseTexture, renderer->getProjectionMatrix(), cloudContainer);
		
		if (recordTimeTaken)
			cloudMarcherShaderTimer->StartTimer();
		shader->compute(renderer->getDeviceContext(), ceil(screenWidth / 4.0f), ceil(screenHeight / 4.0f), 1);
		shader->unbind(renderer->getDeviceContext());
		if (recordTimeTaken)
		{
			cloudMarcherShaderTimer->StopTimer();
			recordTimeTaken = false;
			isTimeRecorded = true;
		}
	}
	else
	{
		shader->setShaderParameters(renderer->getDeviceContext(), sceneRT->getShaderResourceView(), sceneDepthRT->getShaderResourceView(), assets->shapeNoiseGenShader->getSRV(), assets->detailNoiseGenShader->getSRV(), assets->weatherMapShader->getSRV(), assets->blueNoiseTexture, renderer->getProjectionMatrix(), cloudContainer);
		
		if (recordTimeTaken)
			cloudMarcherShaderTimer->StartTimer();
		shader->compute(renderer->getDeviceContext(), ceil(screenWidth / 4.0f), ceil(screenHeight / 4.0f), 1);
		shader->unbind(renderer->getDeviceContext());
		if (recordTimeTaken)
		{
			cloudMarcherShaderTimer->StopTimer();
			recordTimeTaken = false;
			isTimeRecorded = true;
		}
	}
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

	// Render clouds
	assets->screenOrthoMesh->sendData(renderer->getDeviceContext());
	assets->tex2DShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, assets->cloudMarcherShader->getSRV());
	assets->tex2DShader->render(renderer->getDeviceContext(), assets->screenOrthoMesh->getIndexCount());

	// Render ortho mesh with noise texture
	if (showShapeNoiseTexture)
	{
		assets->noiseGenOrthoMesh->sendData(renderer->getDeviceContext());
		assets->tex3DShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, assets->perlinWorleyShader->getSRV(), sliceVal, tileVal);
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
	if (showWeatherMap)
	{
		assets->noiseGenOrthoMesh->sendData(renderer->getDeviceContext());
		assets->tex2DShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, assets->weatherMapShader->getSRV(), 0, tileVal);
		assets->tex2DShader->render(renderer->getDeviceContext(), assets->noiseGenOrthoMesh->getIndexCount());
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

	//ImGui::ShowDemoWindow();

	// Noise settings
	if (ImGui::CollapsingHeader("Noise Settings"))
	{
		ImGui::Checkbox("ShowShapeNoiseTexture", &showShapeNoiseTexture);
		ImGui::Checkbox("ShowDetailNoiseTexture", &showDetailNoiseTexture);
		ImGui::Checkbox("ShowPerlinNoiseTexture", &showPerlinNoiseTexture);
		ImGui::Checkbox("Use Perlin", &usePerlinNoise);
		ImGui::SliderFloat("TileValue", &tileVal, 0, 10);
		ImGui::SliderFloat("Slice", &sliceVal, 0, 1);
	}

	// Cloud settings
	CloudMarcherShader* cloudShader = assets->cloudMarcherShader;
	if (ImGui::CollapsingHeader("Cloud Settings"))
	{
		ImGui::BeginChild("", ImVec2(0, 120), true, ImGuiWindowFlags_None);

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
			ImGui::SliderFloat("Global Coverage", &globalCoverage, 0, 1);
			ImGui::SliderFloat("DensityMultiplier", &densityMultiplier, 1, 10);
			ImGui::SliderInt("DensitySteps", &densitySteps, 0, 1000);
			ImGui::SliderFloat("StepSize", &stepSize, 0, 10);
			ImGui::SliderFloat("EdgeFadePercent", &edgeFadePercent, 0, 1);
		}
		if (ImGui::CollapsingHeader("Optimisation Settings"))
		{
			ImGui::SliderFloat("BlueNoise Strength", &blueNoiseOffsetStrength, 0, 10);
			if (ImGui::Button("Record Compute Time"))
				recordTimeTaken = true;
			ImGui::Text("Compute-time(ms): %.5f", (timetaken * 1000.0));
		}
		cloudShader->SetBlueNoiseStrength(blueNoiseOffsetStrength);

		ImGui::EndChild();
	}
	cloudShader->SetShapeNoiseOffset(shapeNoiseTexOffsetArray[0], shapeNoiseTexOffsetArray[1], shapeNoiseTexOffsetArray[2]);
	cloudShader->SetShapeNoiseScale(shapeNoiseTexScale);
	cloudShader->SetDetailNoiseOffset(detailNoiseTexOffsetArray[0], detailNoiseTexOffsetArray[1], detailNoiseTexOffsetArray[2]);
	cloudShader->SetDetailNoiseScale(detailNoiseTexScale);
	cloudShader->SetDensityThreshold(globalCoverage);
	cloudShader->SetDensityMultiplier(densityMultiplier);
	cloudShader->SetDensitySteps(densitySteps);
	cloudShader->SetStepSize(stepSize);
	cloudShader->SetEdgeFadePercentage(edgeFadePercent);

	// Absorbtion Settings
	if (ImGui::CollapsingHeader("Absoption Settings"))
	{
		ImGui::SliderFloat("AbsTowardsSun", &lightAbsTowardsSun, 0, 1);
		ImGui::SliderFloat("AbsThroughCloud", &lightAbsThroughCloud, 0, 1);
		ImGui::SliderFloat("CloudBrightness", &cloudBrightness, 0, 1);
		ImGui::SliderInt("LightSteps", &lightSteps, 0, 100);
	}
	cloudShader->SetLightAbsTowardsSun(lightAbsTowardsSun);
	cloudShader->SetLightAbsThroughCloud(lightAbsThroughCloud);
	cloudShader->SetCloudBrightness(cloudBrightness);
	cloudShader->SetLightMarchSteps(lightSteps);

	// Light Settings
	if (ImGui::CollapsingHeader("Light Settings"))
	{
		ImGui::ColorPicker3("Colour", lightColour);
		light->setDiffuseColour(lightColour[0], lightColour[1], lightColour[2], 1.0f);
	}

	// Weather Map settings
	if (ImGui::CollapsingHeader("Weather Map Settings"))
	{
		ImGui::Checkbox("Show Weather Map", &showWeatherMap);
		ImGui::SliderFloat("CoverageTexScale", &coverageTexSettings.scale, 1, 1000);
	}
	cloudShader->SetWeatherMapTexSettings(coverageTexSettings, TextureChannel::RED);

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
	assets.perlinWorleyShader = new PerlinWorleyShader(device, hwnd, shapeNoiseGenTexRes, shapeNoiseGenTexRes, shapeNoiseGenTexRes);
	assets.weatherMapShader = new WeatherMapShader(device, hwnd, weatherMapTexRes, weatherMapTexRes);

	// Initialise Meshes
	assets.cubeMesh = new CubeMesh(device, deviceContext);
	assets.planeMesh = new PlaneMesh(device, deviceContext);
	assets.screenOrthoMesh = new OrthoMesh(device, deviceContext, screenWidth, screenHeight);
	assets.noiseGenOrthoMesh = new OrthoMesh(device, deviceContext, shapeNoiseGenTexRes * 2, shapeNoiseGenTexRes * 2);

	// Initialise textures
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");
	textureMgr->loadTexture(L"TerrainColour", L"res/TerrainColour.png");
	textureMgr->loadTexture(L"TerrainHeightMap", L"res/TerrainHeightMap.png");
	textureMgr->loadTexture(L"BlueNoiseTex", L"res/BlueNoiseTex.png");
	assets.brickTexture = textureMgr->getTexture(L"brick");
	assets.terrainColourTexture = textureMgr->getTexture(L"TerrainColour");
	assets.terrainHeightMapTexture = textureMgr->getTexture(L"TerrainHeightMap");
	assets.blueNoiseTexture = textureMgr->getTexture(L"BlueNoiseTex");
}
