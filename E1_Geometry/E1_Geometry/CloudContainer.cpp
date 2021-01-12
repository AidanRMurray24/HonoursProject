#include "CloudContainer.h"
#include "SystemParams.h"
#include "CubeMesh.h"
#include "TextureShader.h"
#include "Assets.h"

CloudContainer::CloudContainer() : SceneObject()
{
	// Set default values
	SetPosition(50, 20, 50);
	SetScale(50, 5, 50);
}

CloudContainer::~CloudContainer()
{
}

void CloudContainer::Render(XMMATRIX view, XMMATRIX proj, ID3D11ShaderResourceView* texture)
{
	ID3D11DeviceContext* deviceContext = SystemParams::GetInstance().GetRenderer()->getDeviceContext();
	Assets assets = SystemParams::GetInstance().GetAssets();
	CubeMesh* mesh = assets.cubeMesh;
	TextureShader* shader = assets.tex2DShader;

	mesh->sendData(deviceContext);
	shader->setShaderParameters(deviceContext, GetTransform(), view, proj, texture);
	shader->render(deviceContext, mesh->getIndexCount());
}
