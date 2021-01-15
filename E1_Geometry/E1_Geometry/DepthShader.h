// Colour shader.h
// Simple shader example.
#pragma once

#include "../DXFramework/DXF.h"

using namespace std;
using namespace DirectX;


class DepthShader : public BaseShader
{
private:
	struct HeightBufferType
	{
		XMFLOAT4 heightMapSettings; // x = hasHeightMap?, y = height
	};

public:

	DepthShader(ID3D11Device* device, HWND hwnd);
	~DepthShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, bool hasHeightMap = false);

private:
	void initShader(const wchar_t* vs, const wchar_t* ps);

private:
	ID3D11Buffer* matrixBuffer;
	ID3D11Buffer* heightBuffer;
};
