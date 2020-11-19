#pragma once
#include "../DXFramework/DXF.h"

using namespace std;
using namespace DirectX;

class SimpleRayMarcherShader : public BaseShader
{
private:
	struct CameraBufferType
	{
		XMMATRIX _CameraToWorld, _CameraInverseProjection;
		XMFLOAT3 _WorldSpaceCameraPos;
		float padding;
	};

public:
	SimpleRayMarcherShader(ID3D11Device* device, HWND hwnd, int w, int h, Camera* cam);
	~SimpleRayMarcherShader();

	void setShaderParameters(ID3D11DeviceContext* dc, ID3D11ShaderResourceView* texture1);
	void createOutputUAV();
	ID3D11ShaderResourceView* getSRV() { return m_srvTexOutput; };
	void unbind(ID3D11DeviceContext* dc);


private:
	void initShader(const wchar_t* cfile, const wchar_t* blank);
	HRESULT CreateStructuredBuffer(ID3D11Device* pDevice, UINT uElementSize, UINT uCount, void* pInitData, ID3D11Buffer** ppBufOut);

	ID3D11ShaderResourceView* srv;
	ID3D11UnorderedAccessView* uav;

	// texture set
	ID3D11Texture2D* m_tex;
	ID3D11UnorderedAccessView* m_uavAccess;
	ID3D11ShaderResourceView* m_srvTexOutput;

	ID3D11Device* deviceObject;

	int sWidth;
	int sHeight;

	Camera* camera;
	ID3D11Buffer* cameraBuffer;
};

