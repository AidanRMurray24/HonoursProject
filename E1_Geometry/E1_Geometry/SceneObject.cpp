#include "SceneObject.h"

SceneObject::SceneObject()
{
	position = XMFLOAT3(0, 0, 0);
	scale = XMFLOAT3(0, 0, 0);
	eulerRotation = XMFLOAT3(0, 0, 0);

	transform = XMMatrixIdentity();
	translationM = XMMatrixIdentity();
	scaleM = XMMatrixIdentity();
	rotationM = XMMatrixIdentity();

	parent = nullptr;
}

SceneObject::~SceneObject()
{
}

void SceneObject::Init()
{
}

void SceneObject::Update(float deltaTime)
{
}

void SceneObject::Render()
{
}

void SceneObject::SetPosition(XMFLOAT3 val)
{
	position = val;
	translationM = XMMatrixTranslation(val.x, val.y, val.z);
	BuildTranformMatrix();
}

void SceneObject::SetPosition(float x, float y, float z)
{
	position = XMFLOAT3(x, y, z);
	translationM = XMMatrixTranslation(x, y, z);
	BuildTranformMatrix();
}

void SceneObject::SetScale(XMFLOAT3 val)
{
	scale = val;
	scaleM = XMMatrixScaling(val.x, val.y, val.z);
	BuildTranformMatrix();
}

void SceneObject::SetScale(float x, float y, float z)
{
	scale = XMFLOAT3(x, y, z);
	scaleM = XMMatrixScaling(x, y, z);
	BuildTranformMatrix();
}

void SceneObject::SetRotation(XMFLOAT3 val)
{
	eulerRotation = val;
	rotationM = XMMatrixRotationRollPitchYaw(val.x, val.y, val.z);
	BuildTranformMatrix();
}

void SceneObject::SetRotation(float x, float y, float z)
{
	eulerRotation = XMFLOAT3(x, y, z);
	rotationM = XMMatrixRotationRollPitchYaw(x, y, z);
	BuildTranformMatrix();
}

void SceneObject::BuildTranformMatrix()
{
	// Calculate the transform of the object depending on whether it has a parent or not
	if (parent != nullptr)
		transform = rotationM * scaleM * translationM * parent->GetTransform();
	else
		transform = rotationM * scaleM * translationM;
}
