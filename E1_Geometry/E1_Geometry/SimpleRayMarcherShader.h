#pragma once
#include "../DXFramework/DXF.h"

using namespace std;
using namespace DirectX;

class SimpleRayMarcherShader : public BaseShader
{
private:
	struct CameraBufferType
	{
		XMMATRIX invViewMatrix, invProjectionMatrix;
		XMFLOAT3 cameraPos;
		float padding;
	};

	struct LightBufferType
	{
		XMFLOAT4 position;
		XMFLOAT4 direction;
		XMFLOAT4 colour;
		XMFLOAT4 intensityTypeAndAngle; // x = intensity, y = type, z = angle
	};

public:
	SimpleRayMarcherShader(ID3D11Device* device, HWND hwnd, int w, int h, Camera* cam, Light* light);
	~SimpleRayMarcherShader();

	void setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* texture1, const XMMATRIX& projectionMatrix);
	void createOutputUAV();
	ID3D11ShaderResourceView* getSRV() { return m_srvTexOutput; };
	void unbind(ID3D11DeviceContext* dc);


private:
	void initShader(const wchar_t* cfile, const wchar_t* blank);
	HRESULT CreateStructuredBuffer(ID3D11Device* pDevice, UINT uElementSize, UINT uCount, void* pInitData, ID3D11Buffer** ppBufOut);

	/*ID3D11ShaderResourceView* srv;
	ID3D11UnorderedAccessView* uav;*/

	// Texture set
	ID3D11Texture2D* m_tex;
	ID3D11UnorderedAccessView* m_uavAccess;
	ID3D11ShaderResourceView* m_srvTexOutput;

	// Screen Info
	int sWidth;
	int sHeight;

	// Buffers
	ID3D11Buffer* cameraBuffer;
	ID3D11Buffer* lightBuffer;
	
	// Scene Info
	Camera* camera;
	Light* light;

	ID3D11Device* deviceObject;
};

