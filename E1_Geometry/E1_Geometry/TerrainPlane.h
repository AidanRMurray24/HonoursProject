#pragma once
#include "SceneObject.h"
#include "Light.h"

class TerrainPlane : public SceneObject
{
public:
	TerrainPlane();
	~TerrainPlane();

	void Render(Light* light);
	void RenderDepthFromCamera();

private:
};

