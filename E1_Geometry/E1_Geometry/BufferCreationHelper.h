#pragma once

#include "../DXFramework/DXF.h"

using namespace std;
using namespace DirectX;

class BufferCreationHelper
{
public:
	static HRESULT CreateStructuredBuffer(ID3D11Device* pDevice, UINT uElementSize, UINT uCount, void* pInitData, ID3D11Buffer** ppBufOut)
	{
		*ppBufOut = nullptr;

		D3D11_BUFFER_DESC desc = {};
		desc.BindFlags = D3D11_BIND_UNORDERED_ACCESS | D3D11_BIND_SHADER_RESOURCE;
		desc.ByteWidth = uElementSize * uCount;
		desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
		desc.StructureByteStride = uElementSize;

		if (pInitData)
		{
			D3D11_SUBRESOURCE_DATA InitData;
			InitData.pSysMem = pInitData;
			return pDevice->CreateBuffer(&desc, &InitData, ppBufOut);
		}
		else
			return pDevice->CreateBuffer(&desc, nullptr, ppBufOut);
	}

	static HRESULT CreateBufferSRV(ID3D11Device* pDevice, ID3D11Buffer* pBuffer, ID3D11ShaderResourceView** ppSRVOut)
	{
		D3D11_BUFFER_DESC descBuf = {};
		pBuffer->GetDesc(&descBuf);

		D3D11_SHADER_RESOURCE_VIEW_DESC desc = {};
		desc.ViewDimension = D3D11_SRV_DIMENSION_BUFFEREX;
		desc.BufferEx.FirstElement = 0;

		if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS)
		{
			// This is a Raw Buffer

			desc.Format = DXGI_FORMAT_R32_TYPELESS;
			desc.BufferEx.Flags = D3D11_BUFFEREX_SRV_FLAG_RAW;
			desc.BufferEx.NumElements = descBuf.ByteWidth / 4;
		}
		else if (descBuf.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED)
		{
			// This is a Structured Buffer

			desc.Format = DXGI_FORMAT_UNKNOWN;
			desc.BufferEx.NumElements = descBuf.ByteWidth / descBuf.StructureByteStride;
		}
		else
		{
			return E_INVALIDARG;
		}

		return pDevice->CreateShaderResourceView(pBuffer, &desc, ppSRVOut);
	}

	static void CreateConstantBuffer(ID3D11Device* renderer, UINT uElementSize, ID3D11Buffer** ppBufOut)
	{
		D3D11_BUFFER_DESC desc;
		desc.Usage = D3D11_USAGE_DYNAMIC;
		desc.ByteWidth = uElementSize;
		desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		desc.MiscFlags = 0;
		desc.StructureByteStride = 0;
		renderer->CreateBuffer(&desc, NULL, ppBufOut);
	}
};
