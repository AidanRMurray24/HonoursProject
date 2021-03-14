#pragma once

#include "BaseShader.h"

using namespace std;
using namespace DirectX;

enum TextureType
{
	TEXTURE2D,
	TEXTURE3D
};

class TextureShader : public BaseShader
{
private:
	struct SliceBufferType
	{
		float sliceNum;
		float tileVal;
		XMFLOAT2 padding;
	};

public:
	TextureShader(ID3D11Device* device, HWND hwnd, TextureType _type);
	~TextureShader();

	void setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &world, const XMMATRIX &view, const XMMATRIX &projection, ID3D11ShaderResourceView* texture, float sliceVal = 0, float tileVal = 1);

private:
	void initShader(const wchar_t*, const wchar_t*);

private:
	ID3D11Buffer * matrixBuffer;
	ID3D11Buffer * sliceBuffer;
	ID3D11SamplerState* sampleState;

	TextureType type;
};

