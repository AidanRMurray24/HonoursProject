#include "CloudContainer.h"
#include "SystemParams.h"
#include "CubeMesh.h"
#include "Assets.h"

// Shaders
#include "TextureShader.h"
#include "DepthShader.h"

CloudContainer::CloudContainer() : SceneObject()
{
	// Set default values
	SetPosition(50, 20, 50);
	SetScale(50, 5, 50);
	SetMesh(SystemParams::GetInstance().GetAssets().cubeMesh);
}

CloudContainer::~CloudContainer()
{
}

void CloudContainer::Render()
{
	ID3D11DeviceContext* deviceContext = SystemParams::GetInstance().GetRenderer()->getDeviceContext();
	XMMATRIX projectionMatrix = SystemParams::GetInstance().GetRenderer()->getProjectionMatrix();
	FPCamera* cam = SystemParams::GetInstance().GetMainCamera();
	Assets& assets = SystemParams::GetInstance().GetAssets();
	ID3D11ShaderResourceView* texture = assets.brickTexture;

	GetMesh()->sendData(deviceContext);
	assets.tex2DShader->setShaderParameters(deviceContext, GetTransform(), cam->getViewMatrix(), projectionMatrix, texture);
	assets.tex2DShader->render(deviceContext, GetMesh()->getIndexCount());
}
