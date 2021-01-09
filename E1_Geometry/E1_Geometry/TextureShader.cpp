#include "TextureShader.h"



TextureShader::TextureShader(ID3D11Device* device, HWND hwnd, TextureType _type) : BaseShader(device, hwnd)
{
	type = _type;

	// Change the shaders used depending on the texture type
	switch (type)
	{
	case TEXTURE2D:
		initShader(L"texture_vs.cso", L"texture_ps.cso");
		break;
	case TEXTURE3D:
		initShader(L"texture3D_vs.cso", L"texture3D_ps.cso");
		break;
	default:
		break;
	}
}


TextureShader::~TextureShader()
{
	// Release the sampler state.
	if (sampleState)
	{
		sampleState->Release();
		sampleState = 0;
	}

	// Release the matrix constant buffer.
	if (matrixBuffer)
	{
		matrixBuffer->Release();
		matrixBuffer = 0;
	}

	// Release the layout.
	if (layout)
	{
		layout->Release();
		layout = 0;
	}

	//Release base shader components
	BaseShader::~BaseShader();
}


void TextureShader::initShader(const wchar_t* vsFilename, const wchar_t* psFilename)
{

	// Load (+ compile) shader files
	loadVertexShader(vsFilename);
	loadPixelShader(psFilename);

	// Setup the description of the dynamic matrix constant buffer that is in the vertex shader.
	D3D11_BUFFER_DESC matrixBufferDesc;
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	// Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	renderer->CreateBuffer(&matrixBufferDesc, NULL, &matrixBuffer);

	// Create a texture sampler state description.
	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	// Create the texture sampler state.
	renderer->CreateSamplerState(&samplerDesc, &sampleState);

	// Setup the slice buffer
	D3D11_BUFFER_DESC sliceBufferDesc;
	sliceBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	sliceBufferDesc.ByteWidth = sizeof(SliceBufferType);
	sliceBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	sliceBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	sliceBufferDesc.MiscFlags = 0;
	sliceBufferDesc.StructureByteStride = 0;
	renderer->CreateBuffer(&sliceBufferDesc, NULL, &sliceBuffer);
}


void TextureShader::setShaderParameters(ID3D11DeviceContext* deviceContext, const XMMATRIX &worldMatrix, const XMMATRIX &viewMatrix, const XMMATRIX &projectionMatrix, ID3D11ShaderResourceView* texture, float sliceVal)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	MatrixBufferType* dataPtr;
	XMMATRIX tworld, tview, tproj;

	// Transpose the matrices to prepare them for the shader.
	tworld = XMMatrixTranspose(worldMatrix);
	tview = XMMatrixTranspose(viewMatrix);
	tproj = XMMatrixTranspose(projectionMatrix);

	// Send matrix data
	result = deviceContext->Map(matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	dataPtr = (MatrixBufferType*)mappedResource.pData;
	dataPtr->world = tworld;// worldMatrix;
	dataPtr->view = tview;
	dataPtr->projection = tproj;
	deviceContext->Unmap(matrixBuffer, 0);
	deviceContext->VSSetConstantBuffers(0, 1, &matrixBuffer);

	// Set shader texture and sampler resource in the pixel shader.
	deviceContext->PSSetShaderResources(0, 1, &texture);
	deviceContext->PSSetSamplers(0, 1, &sampleState);

	// Only pass the slice buffer it the texture is 3D
	if (type == TextureType::TEXTURE3D)
	{
		// Pass the slice data into the pixel shader's constant buffer
		SliceBufferType* slicePtr;
		deviceContext->Map(sliceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		slicePtr = (SliceBufferType*)mappedResource.pData;
		slicePtr->sliceNum = (float)sliceVal;
		deviceContext->Unmap(sliceBuffer, 0);
		deviceContext->PSSetConstantBuffers(0, 1, &sliceBuffer);
	}
}





