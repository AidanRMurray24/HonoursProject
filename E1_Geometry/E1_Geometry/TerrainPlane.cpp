#include "TerrainPlane.h"
#include "SystemParams.h"
#include "Assets.h"

// Shaders
#include "ManipulationShader.h"
#include "DepthShader.h"

TerrainPlane::TerrainPlane() : SceneObject()
{
	SetPosition(0,0,0);
	SetScale(1,1,1);
	SetMesh(SystemParams::GetInstance().GetAssets().planeMesh);
}

TerrainPlane::~TerrainPlane()
{
}

void TerrainPlane::Render(Light* light)
{
	ID3D11DeviceContext* deviceContext = SystemParams::GetInstance().GetRenderer()->getDeviceContext();
	XMMATRIX projectionMatrix = SystemParams::GetInstance().GetRenderer()->getProjectionMatrix();
	FPCamera* cam = SystemParams::GetInstance().GetMainCamera();
	Assets& assets = SystemParams::GetInstance().GetAssets();
	PlaneMesh* mesh = assets.planeMesh;
	ManipulationShader* shader = assets.manipulationShader;
	ID3D11ShaderResourceView* terrainColTexture = assets.terrainColourTexture;
	ID3D11ShaderResourceView* terrainHMTexture = assets.terrainHeightMapTexture;

	GetMesh()->sendData(deviceContext);
	shader->setShaderParameters(deviceContext, GetTransform(), cam->getViewMatrix(), projectionMatrix, terrainColTexture, terrainHMTexture, light);
	shader->render(deviceContext, GetMesh()->getIndexCount());
}

void TerrainPlane::RenderDepthFromCamera()
{
	ID3D11DeviceContext* deviceContext = SystemParams::GetInstance().GetRenderer()->getDeviceContext();
	XMMATRIX projectionMatrix = SystemParams::GetInstance().GetRenderer()->getProjectionMatrix();
	FPCamera* cam = SystemParams::GetInstance().GetMainCamera();
	Assets& assets = SystemParams::GetInstance().GetAssets();

	GetMesh()->sendData(deviceContext);
	assets.depthShader->setShaderParameters(deviceContext, GetTransform(), cam->getViewMatrix(), projectionMatrix, true);
	assets.depthShader->render(deviceContext, GetMesh()->getIndexCount());
}
