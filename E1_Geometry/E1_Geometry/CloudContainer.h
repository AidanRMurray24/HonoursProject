#pragma once
#include "SceneObject.h"

class CloudContainer : public SceneObject
{
public:
	CloudContainer();
	~CloudContainer();

	void Render();

	// Getters
	inline XMFLOAT3 GetBoundsMin() { return boundsMin; }
	inline XMFLOAT3 GetBoundsMax() { return boundsMax; }

private:
	XMFLOAT3 boundsMin;
	XMFLOAT3 boundsMax;
};

