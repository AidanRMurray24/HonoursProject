// Lab1.cpp
// Lab 1 example, simple coloured triangle mesh
#include "App1.h"

App1::App1()
{
	// Shaders
	manipulationShader = nullptr;
	cloudMarcherShader = nullptr;
	rayMarcherShader = nullptr;
	noiseGenShader = nullptr;
	tex2DShader = nullptr;
	tex3DShader = nullptr;

	// Render textures
	sceneRT = nullptr;

	// Meshes
	cubeMesh = nullptr;
	planeMesh = nullptr;
	screenOrthoMesh = nullptr;
	noiseGenOrthoMesh = nullptr;

	// Lights
	light = nullptr;

	// Timers
	noiseTimer = nullptr;
	elapsedTime = 0;
	timetaken = 9;

	// Floats
	noiseGenTexRes = 128;

	// Bools
	textureGenerated = false;
	showWorleyNoiseTexture = false;

	// Sliders
	tileVal = 1.0f;
	sliceVal = 0;
}

void App1::init(HINSTANCE hinstance, HWND hwnd, int _screenWidth, int _screenHeight, Input *in, bool VSYNC, bool FULL_SCREEN)
{
	screenWidth = _screenWidth;
	screenHeight = _screenHeight;

	// Call super/parent init function (required!)
	BaseApplication::init(hinstance, hwnd, screenWidth, screenHeight, in, VSYNC, FULL_SCREEN);

	// Load textures
	textureMgr->loadTexture(L"brick", L"res/brick1.dds");
	textureMgr->loadTexture(L"TerrainColour", L"res/TerrainColour.png");
	textureMgr->loadTexture(L"heightMap", L"res/TerrainHeightMap.png");

	// Initialise Lights
	light = new Light;
	light->setPosition(0, 0, 0);
	light->setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	light->setDirection(0.7f, -0.7f, 0.0f);

	// Initialise Shaders
	manipulationShader = new ManipulationShader(renderer->getDevice(), hwnd);
	rayMarcherShader = new SimpleRayMarcherShader(renderer->getDevice(), hwnd, screenWidth, screenHeight, camera, light);
	tex2DShader = new TextureShader(renderer->getDevice(), hwnd, TextureType::TEXTURE2D);
	tex3DShader = new TextureShader(renderer->getDevice(), hwnd, TextureType::TEXTURE3D);
	noiseGenShader = new NoiseGeneratorShader(renderer->getDevice(), hwnd, noiseGenTexRes, noiseGenTexRes, noiseGenTexRes);
	cloudMarcherShader = new CloudMarcherShader(renderer->getDevice(), hwnd, screenWidth, screenHeight, camera);

	// Initialise Meshes
	cubeMesh = new CubeMesh(renderer->getDevice(), renderer->getDeviceContext());
	planeMesh = new PlaneMesh(renderer->getDevice(), renderer->getDeviceContext());
	screenOrthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), screenWidth, screenHeight);
	noiseGenOrthoMesh = new OrthoMesh(renderer->getDevice(), renderer->getDeviceContext(), noiseGenTexRes, noiseGenTexRes);

	// Initialise timers
	noiseTimer = new GPUTimer(renderer->getDevice(), renderer->getDeviceContext());

	// Initialise Render Textures
	sceneRT = new RenderTexture(renderer->getDevice(), screenWidth, screenHeight, 0.1f, 100.f);
}


App1::~App1()
{
	// Run base application deconstructor
	BaseApplication::~BaseApplication();

	// Release the Direct3D object.

	// Shaders
	{
		if (manipulationShader)
		{
			delete manipulationShader;
			manipulationShader = 0;
		}

		if (rayMarcherShader)
		{
			delete rayMarcherShader;
			rayMarcherShader = 0;
		}

		if (cloudMarcherShader)
		{
			delete cloudMarcherShader;
			cloudMarcherShader = 0;
		}

		if (tex2DShader)
		{
			delete tex2DShader;
			tex2DShader = 0;
		}

		if (tex3DShader)
		{
			delete tex3DShader;
			tex3DShader = 0;
		}

		if (noiseGenShader)
		{
			delete noiseGenShader;
			noiseGenShader = 0;
		}
	}

	// Meshes
	{
		if (planeMesh)
		{
			delete planeMesh;
			planeMesh = 0;
		}

		if (screenOrthoMesh)
		{
			delete screenOrthoMesh;
			screenOrthoMesh = 0;
		}
	}

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
	}

	// Timers
	{
		if (noiseTimer)
		{
			delete noiseTimer;
			noiseTimer = 0;
		}
	}
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
	RayMarchPass();
	CloudMarchPass();
	FinalPass();

	return true;
}

void App1::NoiseGenPass()
{
	// Generate noise texture
	noiseGenShader->setShaderParameters(renderer->getDeviceContext(), tileVal);
	noiseTimer->StartTimer();
	noiseGenShader->compute(renderer->getDeviceContext(), ceil(noiseGenTexRes / 8.0f), ceil(noiseGenTexRes / 8.0f), ceil(noiseGenTexRes / 8.0f));
	noiseGenShader->unbind(renderer->getDeviceContext());
	noiseTimer->StopTimer();
}

void App1::GeometryPass()
{
	// Set the render target to the render texture
	sceneRT->setRenderTarget(renderer->getDeviceContext());
	sceneRT->clearRenderTarget(renderer->getDeviceContext(), 0.39f, 0.58f, 0.92f, 1.0f);

	// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.
	XMMATRIX worldMatrix = renderer->getWorldMatrix();
	XMMATRIX viewMatrix = camera->getViewMatrix();
	XMMATRIX projectionMatrix = renderer->getProjectionMatrix();

	// Render the terrain plane
	planeMesh->sendData(renderer->getDeviceContext());
	manipulationShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, viewMatrix, projectionMatrix, textureMgr->getTexture(L"TerrainColour"), textureMgr->getTexture(L"heightMap"), light);
	manipulationShader->render(renderer->getDeviceContext(), planeMesh->getIndexCount());

	XMMATRIX cubeTransform;
	XMMATRIX translation = XMMatrixTranslation(50,20,50);
	XMMATRIX scale = XMMatrixScaling(50,5,50);
	translation = worldMatrix * translation;
	cubeTransform = scale * translation;

	cubeMesh->sendData(renderer->getDeviceContext());
	tex2DShader->setShaderParameters(renderer->getDeviceContext(), cubeTransform, viewMatrix, projectionMatrix, textureMgr->getTexture(L"brick"));
	tex2DShader->render(renderer->getDeviceContext(), cubeMesh->getIndexCount());

	// Set back buffer as render target and reset view port.
	renderer->setBackBufferRenderTarget();
	renderer->resetViewport();
}

void App1::RayMarchPass()
{
	// Raymarching pass using render texture of the rendered scene
	rayMarcherShader->setShaderParameters(renderer->getDeviceContext(), sceneRT->getShaderResourceView(), renderer->getProjectionMatrix());
	rayMarcherShader->compute(renderer->getDeviceContext(), ceil(sWidth / 8.0f), ceil(sHeight / 8.0f), ceil(sHeight / 8.0f));
	rayMarcherShader->unbind(renderer->getDeviceContext());
}

void App1::CloudMarchPass()
{
	// Generate clouds
	cloudMarcherShader->setShaderParameters(renderer->getDeviceContext(), sceneRT->getShaderResourceView(), renderer->getProjectionMatrix());
	cloudMarcherShader->compute(renderer->getDeviceContext(), ceil(screenWidth / 8.0f), ceil(screenHeight / 8.0f), 1);
	cloudMarcherShader->unbind(renderer->getDeviceContext());
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
	screenOrthoMesh->sendData(renderer->getDeviceContext());
	tex2DShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, rayMarcherShader->getSRV());
	tex2DShader->render(renderer->getDeviceContext(), screenOrthoMesh->getIndexCount());

	// Render clouds
	screenOrthoMesh->sendData(renderer->getDeviceContext());
	tex2DShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, cloudMarcherShader->getSRV());
	tex2DShader->render(renderer->getDeviceContext(), screenOrthoMesh->getIndexCount());

	// Render ortho mesh with noise texture
	if (showWorleyNoiseTexture)
	{
		noiseGenOrthoMesh->sendData(renderer->getDeviceContext());
		tex3DShader->setShaderParameters(renderer->getDeviceContext(), worldMatrix, baseViewMatrix, orthoMatrix, noiseGenShader->getSRV(), sliceVal);
		tex3DShader->render(renderer->getDeviceContext(), noiseGenOrthoMesh->getIndexCount());
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
	ImGui::Text("Noise Compute-time(ms): %.5f", timetaken * 1000);
	ImGui::Checkbox("Wireframe mode", &wireframeToggle);
	ImGui::Checkbox("Show Worley Noise Texture", &showWorleyNoiseTexture);
	ImGui::SliderFloat("Tile Value", &tileVal, 0, 10);
	ImGui::SliderFloat("Slice", &sliceVal, 0, 1);

	// Render UI
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}