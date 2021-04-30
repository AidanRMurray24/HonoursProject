#pragma once
#include "../DXFramework/DXF.h"

class SceneObject
{
public:
	SceneObject();
	virtual ~SceneObject();

	virtual void Init();
	virtual void Update(float deltaTime);
	virtual void Render();


	// Getters
	inline XMFLOAT3 GetPosition() { return position; }
	inline XMFLOAT3 GetScale() { return scale; }
	inline XMFLOAT3 GetEulerRotation() { return scale; }
	inline XMMATRIX GetTransform() { return transform; }
	inline SceneObject* GetParent() { return parent; }
	inline BaseMesh* GetMesh() { return mesh; }

	// Setters
	void SetPosition(XMFLOAT3 val);
	void SetPosition(float x, float y, float z);
	void SetScale(XMFLOAT3 val);
	void SetScale(float x, float y, float z);
	void SetRotation(XMFLOAT3 val);
	void SetRotation(float x, float y, float z);
	inline void SetTransform(XMMATRIX val) { transform = val; }
	inline void SetParent(SceneObject* val) { parent = val; }
	inline void SetMesh(BaseMesh* val) { mesh = val; }

private:
	void BuildTranformMatrix();

	XMFLOAT3 position;
	XMFLOAT3 scale;
	XMFLOAT3 eulerRotation;

	XMMATRIX transform;
	XMMATRIX translationM;
	XMMATRIX scaleM;
	XMMATRIX rotationM;

	SceneObject* parent;
	BaseMesh* mesh;
};

