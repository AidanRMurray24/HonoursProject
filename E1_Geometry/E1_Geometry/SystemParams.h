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
	inline class FPCamera* GetMainCamera() { return mainCamera; }

	// Setters
	inline void SetRenderer(class D3D* val) { renderer = val; }
	inline void SetMainCamera(class FPCamera* val) { mainCamera = val; }

private:
	class D3D* renderer;
	class FPCamera* mainCamera;
	Assets assets;
};

