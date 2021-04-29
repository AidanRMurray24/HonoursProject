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
	SetPosition(50, 100, 50);
	SetScale(300, 100, 300);
	SetMesh(SystemParams::GetInstance().GetAssets().cubeMesh);

	boundsMin = XMFLOAT3(GetPosition().x - GetScale().x, GetPosition().y - GetScale().y, GetPosition().z - GetScale().z);
	boundsMax = XMFLOAT3(GetPosition().x + GetScale().x, GetPosition().y + GetScale().y, GetPosition().z + GetScale().z);
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
