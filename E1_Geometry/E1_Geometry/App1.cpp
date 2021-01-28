// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"
#include "SystemParams.h"

// Shaders
#include "ManipulationShader.h"
#include "SimpleRayMarcherShader.h"
#include "TextureShader.h"
#include "NoiseGeneratorShader.h"
#include "CloudMarcherShader.h"
#include "DepthShader.h"
#include "CloudFragShader.h"

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
	noiseGenTexRes = 128;
	textureGenerated = false;
	showWorleyNoiseTexture = false;
	tileVal = 1.0f;
	sliceVal = 0;

	// Cloud Settings
	densityThreshold = 0.6f;
	densityMultiplier = 1.0f;
	densitySteps = 100;
	noiseTexOffsetArray[0] = 0;
	noiseTexOffsetArray[1] = 0;
	noiseTexOffsetArray[2] = 0;
	noiseTexScale = 40.0f;

	// Absorption settings
	lightAbsTowardsSun = 0.75f;
	lightAbsThroughCloud = 1.21f;
	darknessThreshold = 0.15f;
	lightSteps = 8;

	// Light settings
	lightColour[0] = 1.0f;
	lightColour[1] = 0.9f;
	lightColour[2] = 0.8f;
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
	{
		if (light)
		{
			delete light;
			light = 0;
		}
	}

	// Render Textures
	{
		if (sceneRT)
		{
			delete sceneRT;
			sceneRT = 0;
		}

		if (sceneDepthRT)
		{
			delete sceneDepthRT;
			sceneDepthRT = 0;
		}
	}

	// Timers
	{
		if (noiseTimer)
		{
			delete noiseTimer;
			noiseTimer = 0;
		}
	}

	// Scene Objects
	for (SceneObject* s : sceneObjects)
	{
		if (s)
		{
			delete s;
			s = 0;
		}
	}
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
	if (elapsedTime > 1)
	{
		elapsedTime = 0;
		timetaken = noiseTimer->GetTimeTaken();
	}

	return true;
}

bool App1::render()
{
	//if (!textureGenerated)
	{
		textureGenerated = true;
		NoiseGenPass();
	}
	GeometryPass();
	DepthPass();
	RayMarchPass();
	CloudMarchPass();
	CloudFragPass();
	FinalPass();

	return true;
}

void App1::NoiseGenPass()
{
	NoiseGeneratorShader* shader = SystemParams::GetInstance().GetAssets().noiseGenShader;

	// Generate noise texture
	shader->setShaderParameters(renderer->getDeviceContext(), tileVal);
	noiseTimer->StartTimer();
	shader->compute(renderer->getDeviceContext(), ceil(noiseGenTexRes / 8.0f), ceil(noiseGenTexRes / 8.0f), ceil(noiseGenTexRes / 8.0f));
	shader->unbind(renderer->getDeviceContext());
	noiseTimer->StopTimer();
}

void App1::GeometryPass()
{
	// Set the render target to the render texture
	sceneRT->setRenderTarget(renderer->getDeviceContext());
	sceneRT->clearRenderTarget(renderer->getDeviceContext(), 0.39f, 0.58f, 0.92f, 1.0f);
	
	// Render scene objects
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
	shader->setShaderParameters(renderer->getDeviceContext(), sceneRT->getShaderResourceView(), sceneDepthRT->getShaderResourceView(), assets->noiseGenShader->getSRV(), renderer->getProjectionMatrix(), cloudContainer);
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
	assets->cloudFragShader->setShaderParameters(renderer->getDeviceContext(), sceneRT->getShaderResourceView(), sceneDepthRT->getShaderResourceView(), assets->noiseGenShader->getSRV(), worldMatrix, cloudFragRT->getOrthoMatrix(), cloudContainer);
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
	
	// Render ortho mesh with ray march
	assets->screenOrthoMesh->sendData(renderer->getDeviceContext());
	assets->tex2DShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, assets->rayMarcherShader->getSRV());
	assets->tex2DShader->render(renderer->getDeviceContext(), assets->screenOrthoMesh->getIndexCount());

	// Render ortho mesh with depth map
	assets->screenOrthoMesh->sendData(renderer->getDeviceContext());
	assets->tex2DShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, sceneDepthRT->getShaderResourceView());
	assets->tex2DShader->render(renderer->getDeviceContext(), assets->screenOrthoMesh->getIndexCount());

	// Render clouds
	assets->screenOrthoMesh->sendData(renderer->getDeviceContext());
	assets->tex2DShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, assets->cloudMarcherShader->getSRV());
	assets->tex2DShader->render(renderer->getDeviceContext(), assets->screenOrthoMesh->getIndexCount());

	//// Render frag clouds
	//assets->screenOrthoMesh->sendData(renderer->getDeviceContext());
	//assets->tex2DShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, cloudFragRT->getShaderResourceView());
	//assets->tex2DShader->render(renderer->getDeviceContext(), assets->screenOrthoMesh->getIndexCount());

	// Render ortho mesh with noise texture
	if (showWorleyNoiseTexture)
	{
		assets->noiseGenOrthoMesh->sendData(renderer->getDeviceContext());
		assets->tex3DShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, assets->noiseGenShader->getSRV(), sliceVal);
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

	// Noise settings
	if (ImGui::CollapsingHeader("Noise Settings"))
	{
		ImGui::Text("Noise Compute-time(ms): %.5f", timetaken * 1000);
		ImGui::Checkbox("ShowWorleyNoiseTexture", &showWorleyNoiseTexture);
		ImGui::SliderFloat("TileValue", &tileVal, 0, 10);
		ImGui::SliderFloat("Slice", &sliceVal, 0, 1);
	}

	// Cloud settings
	CloudMarcherShader* cloudShader = assets->cloudMarcherShader;
	if (ImGui::CollapsingHeader("Cloud Settings"))
	{
		ImGui::SliderFloat3("NoiseOffset", noiseTexOffsetArray, 0, 100);
		ImGui::SliderFloat("NoiseTexture Scale", &noiseTexScale, 1, 200);
		ImGui::SliderFloat("DensityThreshold", &densityThreshold, 0, 1);
		ImGui::SliderFloat("DensityMultiplier", &densityMultiplier, 1, 10);
		ImGui::SliderInt("DensitySteps", &densitySteps, 0, 1000);
	}
	cloudShader->SetNoiseOffset(noiseTexOffsetArray[0], noiseTexOffsetArray[1], noiseTexOffsetArray[2]);
	cloudShader->SetNoiseScale(noiseTexScale);
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
	assets.noiseGenShader = new NoiseGeneratorShader(device, hwnd, noiseGenTexRes, noiseGenTexRes, noiseGenTexRes);
	assets.cloudMarcherShader = new CloudMarcherShader(device, hwnd, screenWidth, screenHeight, camera, light);
	assets.depthShader = new DepthShader(device, hwnd);
	assets.cloudFragShader = new CloudFragShader(device, hwnd, screenWidth, screenHeight, camera);

	// Initialise Meshes
	assets.cubeMesh = new CubeMesh(device, deviceContext);
	assets.planeMesh = new PlaneMesh(device, deviceContext);
	assets.screenOrthoMesh = new OrthoMesh(device, deviceContext, screenWidth, screenHeight);
	assets.noiseGenOrthoMesh = new OrthoMesh(device, deviceContext, noiseGenTexRes, noiseGenTexRes);

	// Initialise textures
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");
	textureMgr->loadTexture(L"TerrainColour", L"res/TerrainColour.png");
	textureMgr->loadTexture(L"TerrainHeightMap", L"res/TerrainHeightMap.png");
	assets.brickTexture = textureMgr->getTexture(L"brick");
	assets.terrainColourTexture = textureMgr->getTexture(L"TerrainColour");
	assets.terrainHeightMapTexture = textureMgr->getTexture(L"TerrainHeightMap");
}
