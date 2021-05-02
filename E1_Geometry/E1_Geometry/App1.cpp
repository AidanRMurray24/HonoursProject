// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"
#include "SystemParams.h"

// Shaders
#include "ManipulationShader.h"
#include "TextureShader.h"
#include "WorleyNoiseShader.h"
#include "CloudMarcherShader.h"
#include "PerlinNoiseShader.h"
#include "PerlinWorleyShader.h"
#include "WeatherMapShader.h"
#include "TemporalReprojectionShader.h"

App1::App1()
{
	assets = nullptr;
	cloudContainer = nullptr;
	terrainPlane = nullptr;
	currentInvViewProjMatrix = XMMatrixIdentity();
	oldViewProjMatrix = XMMatrixIdentity();
	screenHeight = 0;
	screenWidth = 0;

	// Render textures
	sceneRT = nullptr;
	cloudFragRT = nullptr;

	// Lights
	light = nullptr;

	// Timers
	cloudMarcherShaderTimer = nullptr;

	// Noise data
	shapeNoiseGenTexRes = 128;
	detailNoiseGenTexRes = 32;
	textureGenerated = false;
	showShapeNoiseTexture = false;
	showDetailNoiseTexture = false;
	showPerlinNoiseTexture = false;
	usePerlinNoise = true;
	tileVal = 1.0f;
	sliceVal = 0;
	blueNoiseOffsetStrength = 6.f;
	noiseOrthoMeshRes = 256;
	shapeWeights[0] = 0;
	shapeWeights[1] = 0;
	shapeWeights[2] = 0;
	shapeWeights[3] = 0;
	detailWeights[0] = 0;
	detailWeights[1] = 0;
	detailWeights[2] = 0;

	// Cloud Settings
	globalCoverage = 0.5f;
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
	useTemporalReprojection = false;
	cloudTextureRes = XMFLOAT2(0,0);

	// Absorption settings
	lightAbsTowardsSun = 0.4f;
	lightAbsThroughCloud = 0.84f;
	cloudBrightness = 0.7f;
	lightSteps = 4;

	// Light settings
	lightColour[0] = 1;
	lightColour[1] = 1;
	lightColour[2] = 1;

	// Weather map settings
	showWeatherMap = false;
	weatherMapTexRes = 128;
	weatherChannelIntensities[0] = 0.5f;
	weatherChannelIntensities[1] = 0.5f;

	// Terrain settings
	showTerrain = false;

	// Testing
	elapsedTime = 0;
	timetaken = 0;
	recordTimeTaken = false;
	isTimeRecorded = false;
	testStarted = false;
	testFinished = false;
	currentTest = nullptr;
	coverageTest = nullptr;
	distanceTest = nullptr;
	stepSizeTest = nullptr;
	lightStepsTest = nullptr;
	estimatedTimeRemaining = 0;
	timeBetweenRecordings = 0.4f;
	numRecordingToAverage = 10;
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int _screenWidth, int _screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	screenWidth = _screenWidth;
	screenHeight = _screenHeight;

	cloudTextureRes = XMFLOAT2(screenWidth / 2, screenHeight / 2);

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
	lightColour[0] = light->getDiffuseColour().x;
	lightColour[1] = light->getDiffuseColour().y;
	lightColour[2] = light->getDiffuseColour().z;

	LoadAssets(hwnd);

	// Set shape noise settings
	{
		// RED
		shapeNoiseSettings.seed = 34;
		shapeNoiseSettings.numCellsA = 7;
		shapeNoiseSettings.numCellsB = 12;
		shapeNoiseSettings.numCellsC = 17;
		shapeNoiseSettings.persistence = 0.5f;
		shapeNoiseSettings.channel = XMFLOAT4(1, 0, 0, 0);
		assets->shapeNoiseGenShader->SetNoiseSettings(shapeNoiseSettings, TextureChannel::RED);

		// GREEN
		shapeNoiseSettings.seed = 23;
		shapeNoiseSettings.numCellsA = 10;
		shapeNoiseSettings.numCellsB = 20;
		shapeNoiseSettings.numCellsC = 30;
		shapeNoiseSettings.persistence = 0.5f;
		shapeNoiseSettings.channel = XMFLOAT4(0, 1, 0, 0);
		assets->shapeNoiseGenShader->SetNoiseSettings(shapeNoiseSettings, TextureChannel::GREEN);

		// BLUE
		shapeNoiseSettings.seed = 456;
		shapeNoiseSettings.numCellsA = 20;
		shapeNoiseSettings.numCellsB = 40;
		shapeNoiseSettings.numCellsC = 60;
		shapeNoiseSettings.persistence = 0.5f;
		shapeNoiseSettings.channel = XMFLOAT4(0, 0, 1, 0);
		assets->shapeNoiseGenShader->SetNoiseSettings(shapeNoiseSettings, TextureChannel::BLUE);

		// ALPHA
		shapeNoiseSettings.seed = 656;
		shapeNoiseSettings.numCellsA = 40;
		shapeNoiseSettings.numCellsB = 80;
		shapeNoiseSettings.numCellsC = 100;
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
	shapeWeights[0] = 1;
	shapeWeights[1] = 0.625f;
	shapeWeights[2] = 0.25f;
	shapeWeights[3] = 0.125f;
	assets->cloudMarcherShader->SetShapeNoiseWeights(XMFLOAT4(shapeWeights[0], shapeWeights[1], shapeWeights[2], shapeWeights[3]));
	detailWeights[0] = 0.625f;
	detailWeights[1] = 0.25f;
	detailWeights[2] = 0.125f;
	assets->cloudMarcherShader->SetDetailNoiseWeights(XMFLOAT4(detailWeights[0], detailWeights[1], detailWeights[2], 0.f));

	// Initialise timers
	cloudMarcherShaderTimer = new GPUTimer(renderer->getDevice(), renderer->getDeviceContext());

	// Initialise Render Textures
	sceneRT = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, 0.1f, 1000.f);
	cloudFragRT = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, 0.1f, 1000.f);

	// Initialise scene objects
	cloudContainer = new CloudContainer();
	sceneObjects.push_back(cloudContainer);
	terrainPlane = new TerrainPlane();
	sceneObjects.push_back(terrainPlane);

	oldViewProjMatrix = XMMatrixMultiply(camera->getViewMatrix(), renderer->getProjectionMatrix());

	// Initailise tests
	coverageTest = new CoverageTest(&globalCoverage, camera, cloudContainer);
	distanceTest = new DistanceTest(camera, cloudContainer);
	stepSizeTest = new StepSizeTest(&stepSize, camera, cloudContainer);
	lightStepsTest = new LightStepsTest(&lightSteps, camera, cloudContainer);
	reprojectionTest = new ReprojectionTest(&useTemporalReprojection, camera, cloudContainer);
	currentTest = coverageTest;
}


App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Lights
	CLEAN_POINTER(light);

	// Render Textures
	CLEAN_POINTER(sceneRT);

	// Timers
	CLEAN_POINTER(cloudMarcherShaderTimer);

	// Tests
	CLEAN_POINTER(coverageTest);
	CLEAN_POINTER(distanceTest);
	CLEAN_POINTER(stepSizeTest);
	CLEAN_POINTER(lightStepsTest);
	CLEAN_POINTER(reprojectionTest);

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

	// If testing has started, start recording the compute time until all entries have been recorded
	if (testStarted && isTimeRecorded)
	{
		estimatedTimeRemaining -= timer->getTime();
		elapsedTime += timer->getTime();
		if (elapsedTime > timeBetweenRecordings)
		{
			elapsedTime = 0;
			isTimeRecorded = false;
			computeTimes.push_back(cloudMarcherShaderTimer->GetTimeTaken());
			if (computeTimes.size() < numRecordingToAverage)
				recordTimeTaken = true;
			else
			{
				// Calculate the average compute time
				float averageComputeTime = 0;
				for (size_t i = 0; i < computeTimes.size(); i++)
					averageComputeTime += computeTimes.at(i);
				averageComputeTime /= computeTimes.size();

				// Clear the compute times list
				computeTimes.clear();
				
				// Add an entry to the preformance test list and check if we are done testing
				testFinished = currentTest->UpdateEntries(averageComputeTime);
				if (testFinished)
					testStarted = false;
				else
					recordTimeTaken = true;
			}
		}
	}

	// Record the compute time of the cloud marcher shader
	if (isTimeRecorded && !testStarted)
	{
		elapsedTime += timer->getTime();
		if (elapsedTime > timeBetweenRecordings)
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
	if (useTemporalReprojection)
		ReprojectionPass();
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
	WorleyNoiseShader* shapeNoiseShader = SystemParams::GetInstance().GetAssets().shapeNoiseGenShader;
	shapeNoiseShader->setShaderParameters(renderer->getDeviceContext(), TextureChannel::RED);
	shapeNoiseShader->compute(renderer->getDeviceContext(), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f));
	shapeNoiseShader->unbind(renderer->getDeviceContext());
	shapeNoiseShader->setShaderParameters(renderer->getDeviceContext(), TextureChannel::GREEN);
	shapeNoiseShader->compute(renderer->getDeviceContext(), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f));
	shapeNoiseShader->unbind(renderer->getDeviceContext());
	shapeNoiseShader->setShaderParameters(renderer->getDeviceContext(), TextureChannel::BLUE);
	shapeNoiseShader->compute(renderer->getDeviceContext(), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f));
	shapeNoiseShader->unbind(renderer->getDeviceContext());
	shapeNoiseShader->setShaderParameters(renderer->getDeviceContext(), TextureChannel::ALPHA);
	shapeNoiseShader->compute(renderer->getDeviceContext(), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f));
	shapeNoiseShader->unbind(renderer->getDeviceContext());

	// Generate detail noise texture
	WorleyNoiseShader* detailNoiseShader = SystemParams::GetInstance().GetAssets().detailNoiseGenShader;
	detailNoiseShader->setShaderParameters(renderer->getDeviceContext(), TextureChannel::RED);
	detailNoiseShader->compute(renderer->getDeviceContext(), ceil(detailNoiseGenTexRes / 8.0f), ceil(detailNoiseGenTexRes / 8.0f), ceil(detailNoiseGenTexRes / 8.0f));
	detailNoiseShader->unbind(renderer->getDeviceContext());
	detailNoiseShader->setShaderParameters(renderer->getDeviceContext(), TextureChannel::GREEN);
	detailNoiseShader->compute(renderer->getDeviceContext(), ceil(detailNoiseGenTexRes / 8.0f), ceil(detailNoiseGenTexRes / 8.0f), ceil(detailNoiseGenTexRes / 8.0f));
	detailNoiseShader->unbind(renderer->getDeviceContext());
	detailNoiseShader->setShaderParameters(renderer->getDeviceContext(), TextureChannel::BLUE);
	detailNoiseShader->compute(renderer->getDeviceContext(), ceil(detailNoiseGenTexRes / 8.0f), ceil(detailNoiseGenTexRes / 8.0f), ceil(detailNoiseGenTexRes / 8.0f));
	detailNoiseShader->unbind(renderer->getDeviceContext());

	// Generate perlin noise texture
	PerlinNoiseShader* perlinNoiseShader = SystemParams::GetInstance().GetAssets().perlinNoiseShader;
	perlinNoiseShader->setShaderParameters(renderer->getDeviceContext());
	perlinNoiseShader->compute(renderer->getDeviceContext(), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f), ceil(shapeNoiseGenTexRes / 8.0f));
	perlinNoiseShader->unbind(renderer->getDeviceContext());

	// Combine the perlin and worley textures
	PerlinWorleyShader* perlinWorleyShader = SystemParams::GetInstance().GetAssets().perlinWorleyShader;
	perlinWorleyShader->setShaderParameters(renderer->getDeviceContext(), perlinNoiseShader->getSRV(), shapeNoiseShader->getSRV());
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

void App1::CloudMarchPass()
{
	CloudMarcherShader* shader = SystemParams::GetInstance().GetAssets().cloudMarcherShader;

	// Generate clouds
	if (usePerlinNoise)
	{
		shader->setShaderParameters(renderer->getDeviceContext(), sceneRT->getShaderResourceView(), assets->perlinWorleyShader->getSRV(), assets->detailNoiseGenShader->getSRV(), assets->weatherMapShader->getSRV(), assets->blueNoiseTexture, renderer->getProjectionMatrix(), cloudContainer);
		
		if (recordTimeTaken)
			cloudMarcherShaderTimer->StartTimer();
		shader->compute(renderer->getDeviceContext(), ceil((int)(cloudTextureRes.x / 4.0f)), ceil((int)(cloudTextureRes.y / 4.0f)), 1);
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
		shader->setShaderParameters(renderer->getDeviceContext(), sceneRT->getShaderResourceView(), assets->shapeNoiseGenShader->getSRV(), assets->detailNoiseGenShader->getSRV(), assets->weatherMapShader->getSRV(), assets->blueNoiseTexture, renderer->getProjectionMatrix(), cloudContainer);
		
		if (recordTimeTaken)
			cloudMarcherShaderTimer->StartTimer();
		shader->compute(renderer->getDeviceContext(), ceil((int)(cloudTextureRes.x / 4.0f)), ceil((int)(cloudTextureRes.y / 4.0f)), 1);
		shader->unbind(renderer->getDeviceContext());
		if (recordTimeTaken)
		{
			cloudMarcherShaderTimer->StopTimer();
			recordTimeTaken = false;
			isTimeRecorded = true;
		}
	}
}

void App1::ReprojectionPass()
{
	TemporalReprojectionShader* shader = assets->temporalReprojectionShader;
	currentInvViewProjMatrix = XMMatrixInverse(nullptr, XMMatrixMultiply(camera->getViewMatrix(), renderer->getProjectionMatrix()));

	shader->setShaderParameters(renderer->getDeviceContext(), assets->cloudMarcherShader->getSRV(), assets->cloudMarcherShader->getPreviousTex(), XMMatrixTranspose(oldViewProjMatrix), XMMatrixTranspose(currentInvViewProjMatrix), reprojectionFrameCounter);
	shader->compute(renderer->getDeviceContext(), ceil((int)(cloudTextureRes.x / 4.0f)), ceil((int)(cloudTextureRes.y / 4.0f)), 1);
	shader->unbind(renderer->getDeviceContext());

	oldViewProjMatrix = XMMatrixMultiply(camera->getViewMatrix(), renderer->getProjectionMatrix());
	assets->cloudMarcherShader->SaveLastFrame(renderer->getDeviceContext());
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

	// Render temporal reprojection pass
	if (useTemporalReprojection)
	{
		assets->screenOrthoMesh->sendData(renderer->getDeviceContext());
		assets->tex2DShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, assets->temporalReprojectionShader->getSRV());
		assets->tex2DShader->render(renderer->getDeviceContext(), assets->screenOrthoMesh->getIndexCount());
	}

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
	//ImGui::Checkbox("Wireframe mode", &wireframeToggle);
	ImGui::Checkbox("Show Terrain", &showTerrain);

	//ImGui::ShowDemoWindow();
	
	CloudMarcherShader* cloudShader = assets->cloudMarcherShader;

	// Noise settings
	if (ImGui::CollapsingHeader("Noise Settings"))
	{
		ImGui::BeginChild("1", ImVec2(0, 150), true, ImGuiWindowFlags_None);

		ImGui::Checkbox("ShowShapeNoiseTexture", &showShapeNoiseTexture);
		ImGui::Checkbox("ShowDetailNoiseTexture", &showDetailNoiseTexture);
		ImGui::Checkbox("ShowPerlinNoiseTexture", &showPerlinNoiseTexture);
		ImGui::Checkbox("Use Perlin", &usePerlinNoise);
		ImGui::SliderFloat("TileValue", &tileVal, 0, 10);
		ImGui::SliderFloat("Slice", &sliceVal, 0, 1);
		ImGui::SliderFloat4("Shape Noise Weights", shapeWeights, 0, 1);
		ImGui::SliderFloat3("Detail Noise Weights", detailWeights, 0, 1);
		ImGui::EndChild();
	}
	cloudShader->SetShapeNoiseWeights(XMFLOAT4(shapeWeights[0], shapeWeights[1], shapeWeights[2], shapeWeights[3]));
	cloudShader->SetDetailNoiseWeights(XMFLOAT4(detailWeights[0], detailWeights[1], detailWeights[2], 0.f));

	// Cloud settings
	if (ImGui::CollapsingHeader("Cloud Settings"))
	{
		ImGui::BeginChild("2", ImVec2(0, 150), true, ImGuiWindowFlags_None);

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
			ImGui::SliderFloat("EdgeFadePercent", &edgeFadePercent, 0, 1);
		}
		ImGui::EndChild();
	}
	cloudShader->SetShapeNoiseOffset(shapeNoiseTexOffsetArray[0], shapeNoiseTexOffsetArray[1], shapeNoiseTexOffsetArray[2]);
	cloudShader->SetShapeNoiseScale(shapeNoiseTexScale);
	cloudShader->SetDetailNoiseOffset(detailNoiseTexOffsetArray[0], detailNoiseTexOffsetArray[1], detailNoiseTexOffsetArray[2]);
	cloudShader->SetDetailNoiseScale(detailNoiseTexScale);
	cloudShader->SetDensityThreshold(globalCoverage);
	cloudShader->SetDensityMultiplier(densityMultiplier);
	cloudShader->SetEdgeFadePercentage(edgeFadePercent);

	// Light Settings
	if (ImGui::CollapsingHeader("Light Settings"))
	{
		ImGui::BeginChild("3", ImVec2(0, 150), true, ImGuiWindowFlags_None);

		ImGui::SliderFloat("CloudBrightness", &cloudBrightness, 0, 1);

		if (ImGui::CollapsingHeader("Colour"))
		{
			ImGui::ColorPicker3("Colour", lightColour);
			light->setDiffuseColour(lightColour[0], lightColour[1], lightColour[2], 1.0f);
		}

		// Absorbtion Settings
		if (ImGui::CollapsingHeader("Absoption Settings"))
		{
			ImGui::SliderFloat("AbsTowardsSun", &lightAbsTowardsSun, 0, 1);
			ImGui::SliderFloat("AbsThroughCloud", &lightAbsThroughCloud, 0, 1);
		}

		// Scattering
		if (ImGui::CollapsingHeader("Scatter Settings"))
		{
			ImGui::SliderFloat("In Scatter", &scatterSettings.inScatter, 0, 1);
			ImGui::SliderFloat("Out Scatter", &scatterSettings.outScatter, 0, 1);
			ImGui::SliderFloat("In Out Blend", &scatterSettings.inOutScatterBlend, 0, 1);
			ImGui::SliderFloat("Silver Lining Intensity", &scatterSettings.silverLiningIntensity, 0, 50);
			ImGui::SliderFloat("Silver Lining Exponent", &scatterSettings.silverLiningExponent, 0, 50);
		}

		ImGui::EndChild();
	}
	cloudShader->SetLightAbsTowardsSun(lightAbsTowardsSun);
	cloudShader->SetLightAbsThroughCloud(lightAbsThroughCloud);
	cloudShader->SetCloudBrightness(cloudBrightness);
	cloudShader->SetScatterSettings(scatterSettings);

	// Weather Map settings
	if (ImGui::CollapsingHeader("Weather Map Settings"))
	{
		ImGui::Checkbox("Show Weather Map", &showWeatherMap);
		ImGui::SliderFloat("CoverageTexScale(RED)", &coverageTexSettings.scale, 1, 1000);
		ImGui::SliderFloat("HeightTexScale(GREEN)", &heightTexSettings.scale, 1, 1000);
		ImGui::SliderFloat2("Channel Intensities", weatherChannelIntensities, 0, 1);
	}
	coverageTexSettings.intensity = weatherChannelIntensities[0];
	heightTexSettings.intensity = weatherChannelIntensities[1];
	cloudShader->SetWeatherMapTexSettings(coverageTexSettings, TextureChannel::RED);
	cloudShader->SetWeatherMapTexSettings(heightTexSettings, TextureChannel::GREEN);

	// Optimisation settings
	if (ImGui::CollapsingHeader("Optimisation Settings"))
	{
		ImGui::SliderInt("DensitySteps", &densitySteps, 0, 1000);
		ImGui::SliderInt("LightSteps", &lightSteps, 0, 100);
		ImGui::SliderFloat("StepSize", &stepSize, 0, 10);
		ImGui::SliderFloat("BlueNoise Strength", &blueNoiseOffsetStrength, 0, 10);
		ImGui::Checkbox("Temporal Reprojection", &useTemporalReprojection);
	}
	cloudShader->SetDensitySteps(densitySteps);
	cloudShader->SetLightMarchSteps(lightSteps);
	cloudShader->SetStepSize(stepSize);
	cloudShader->SetBlueNoiseStrength(blueNoiseOffsetStrength);
	cloudShader->SetTemporalReprojection(useTemporalReprojection);

	// Testing
	if (ImGui::CollapsingHeader("Testing"))
	{
		if (ImGui::Button("Display Compute Time"))
			recordTimeTaken = true;
		ImGui::Text("Compute-time(ms): %.5f", timetaken);

		if (ImGui::Button("Record Coverage Times"))
			StartTest(coverageTest);

		if (ImGui::Button("Record Distance Times"))
			StartTest(distanceTest);

		if (ImGui::Button("Record Step Size Times"))
			StartTest(stepSizeTest);

		if (ImGui::Button("Record Light Step Times"))
			StartTest(lightStepsTest);

		if (ImGui::Button("Record Temporal Reprojection Times"))
			StartTest(reprojectionTest);

		//// Show the estimated time remaining if the test has started
		//if (testStarted)
		//{
		//	ImGui::Separator();
		//	ImGui::Text("Estimated Time Remaining(s): %.2f", estimatedTimeRemaining);
		//}
	}

	// Show the estimated time remaining if the test has started
	if (testStarted)
	{
		ImVec2 centre(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
		ImGui::SetNextWindowPos(centre, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

		if (ImGui::BeginPopupModal("Testing", NULL, ImGuiWindowFlags_AlwaysAutoResize))
		{
			ImGui::Text("Estimated Time Remaining(s): %.2f", estimatedTimeRemaining);
			ImGui::Separator();

			if (ImGui::Button("Cancel", ImVec2(120, 0)))
			{
				ImGui::CloseCurrentPopup();
				CancelTest();
			}
			ImGui::SetItemDefaultFocus();
			ImGui::EndPopup();
		}
	}	

	if (testFinished)
	{
		ImGui::CloseCurrentPopup();
		testFinished = false;
		ImGui::OpenPopup("Notice");
	}
	DrawMessageBox("Notice", "Finished Testing!", ImVec4(0, 1, 0, 1));

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
	assets.tex2DShader = new TextureShader(device, hwnd, TextureType::TEXTURE2D);
	assets.tex3DShader = new TextureShader(device, hwnd, TextureType::TEXTURE3D);
	assets.shapeNoiseGenShader = new WorleyNoiseShader(device, hwnd, shapeNoiseGenTexRes, shapeNoiseGenTexRes, shapeNoiseGenTexRes);
	assets.detailNoiseGenShader = new WorleyNoiseShader(device, hwnd, detailNoiseGenTexRes, detailNoiseGenTexRes, detailNoiseGenTexRes);
	assets.cloudMarcherShader = new CloudMarcherShader(device, hwnd, cloudTextureRes.x, cloudTextureRes.y, camera, light);
	assets.perlinNoiseShader = new PerlinNoiseShader(device, hwnd, shapeNoiseGenTexRes, shapeNoiseGenTexRes, shapeNoiseGenTexRes);
	assets.perlinWorleyShader = new PerlinWorleyShader(device, hwnd, shapeNoiseGenTexRes, shapeNoiseGenTexRes, shapeNoiseGenTexRes);
	assets.weatherMapShader = new WeatherMapShader(device, hwnd, weatherMapTexRes, weatherMapTexRes);
	assets.temporalReprojectionShader = new TemporalReprojectionShader(device, hwnd, cloudTextureRes.x, cloudTextureRes.y);

	// Initialise Meshes
	assets.cubeMesh = new CubeMesh(device, deviceContext);
	assets.planeMesh = new PlaneMesh(device, deviceContext);
	assets.screenOrthoMesh = new OrthoMesh(device, deviceContext, screenWidth, screenHeight);
	assets.noiseGenOrthoMesh = new OrthoMesh(device, deviceContext, noiseOrthoMeshRes, noiseOrthoMeshRes);

	// Initialise textures
	textureMgr->loadTexture(L"TerrainColour", L"res/TerrainColour.png");
	textureMgr->loadTexture(L"TerrainHeightMap", L"res/TerrainHeightMap.png");
	textureMgr->loadTexture(L"BlueNoiseTex", L"res/BlueNoiseTex.png");
	assets.terrainColourTexture = textureMgr->getTexture(L"TerrainColour");
	assets.terrainHeightMapTexture = textureMgr->getTexture(L"TerrainHeightMap");
	assets.blueNoiseTexture = textureMgr->getTexture(L"BlueNoiseTex");
}

void App1::DrawMessageBox(std::string windowTag, std::string message, ImVec4 messageColour)
{
	ImVec2 centre(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
	ImGui::SetNextWindowPos(centre, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

	if (ImGui::BeginPopupModal(windowTag.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::TextColored(messageColour, "%s\n\n", message.c_str());
		ImGui::Separator();

		if (ImGui::Button("Okay", ImVec2(120, 0)))
			ImGui::CloseCurrentPopup();
		ImGui::SetItemDefaultFocus();
		ImGui::EndPopup();
	}
}

void App1::StartTest(PerformanceTest* test)
{
	// Return if a test has already started
	if (testStarted)
		return;

	testStarted = true;
	recordTimeTaken = true;
	currentTest = test;
	currentTest->StartTest();
	estimatedTimeRemaining = currentTest->GetEstimatedTimeToComplete();
	ImGui::OpenPopup("Testing");
}

void App1::CancelTest()
{
	currentTest->CancelTest();
	testStarted = false;
	testFinished = false;
	recordTimeTaken = false;
	isTimeRecorded = false;
}
