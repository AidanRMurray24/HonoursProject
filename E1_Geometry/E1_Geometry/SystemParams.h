#pragma once
#include "Assets.h"


class SystemParams
{
private:
	SystemParams();

public:
	// Singleton Pattern
	static SystemParams& GetInstance()
	{
		static SystemParams INSTANCE;
		return INSTANCE;
	}
	void CleanUp();

	// Getters
	inline Assets& GetAssets() { return assets; }
	inline class D3D* GetRenderer() { return renderer; }

	// Setters
	inline void SetRenderer(class D3D* val) { renderer = val; }

private:
	class D3D* renderer;
	Assets assets;
};

