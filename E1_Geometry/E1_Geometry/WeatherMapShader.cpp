#include "WeatherMapShader.h"
#include "BufferCreationHelper.h"

WeatherMapShader::WeatherMapShader(ID3D11Device* _device, HWND hwnd, int w, int h) : BaseShader(_device, hwnd)
{
	device = _device;
	texWidth = w;
	texHeight = h;
	initShader(L"weatherMap_cs.cso", NULL);
}

WeatherMapShader::~WeatherMapShader()
{
}

void WeatherMapShader::setShaderParameters(ID3D11DeviceContext* dc)
{
	// Set UAVs
	dc->CSSetUnorderedAccessViews(0, 1, &m_uavAccess, 0);

	// Set SRV
	dc->CSSetShaderResources(0, 1, &permutationBufferSRV);
}

void WeatherMapShader::createGPUViews()
{
	// Setup the description for the 2D texture
	D3D11_TEXTURE2D_DESC textureDesc2D;
	ZeroMemory(&textureDesc2D, sizeof(textureDesc2D));
	textureDesc2D.Width = texWidth;
	textureDesc2D.Height = texHeight;
	textureDesc2D.MipLevels = 1;
	textureDesc2D.ArraySize = 1;
	textureDesc2D.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc2D.SampleDesc.Count = 1;
	textureDesc2D.SampleDesc.Quality = 0;
	textureDesc2D.Usage = D3D11_USAGE_DEFAULT;
	textureDesc2D.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_UNORDERED_ACCESS;
	textureDesc2D.CPUAccessFlags = 0;
	textureDesc2D.MiscFlags = 0;
	tex2D = 0;
	renderer->CreateTexture2D(&textureDesc2D, 0, &tex2D);

	// Setup UAV for the output texture
	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMemory(&uavDesc, sizeof(uavDesc));
	uavDesc.Format = textureDesc2D.Format;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_TEXTURE2D;
	uavDesc.Texture2D.MipSlice = 0;
	renderer->CreateUnorderedAccessView(tex2D, &uavDesc, &m_uavAccess);

	// Setup SRV for the output texture
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMemory(&srvDesc, sizeof(srvDesc));
	srvDesc.Format = textureDesc2D.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = 1;
	renderer->CreateShaderResourceView(tex2D, &srvDesc, &m_srvTexOutput);
}

void WeatherMapShader::unbind(ID3D11DeviceContext* dc)
{
	ID3D11ShaderResourceView* nullSRV[] = { NULL };
	dc->CSSetShaderResources(0, 1, nullSRV);

	// Unbind output from compute shader
	ID3D11UnorderedAccessView* nullUAV[] = { NULL };
	dc->CSSetUnorderedAccessViews(0, 1, nullUAV, 0);

	// Disable Compute Shader
	dc->CSSetShader(nullptr, nullptr, 0);
}

void WeatherMapShader::initShader(const wchar_t* cfile, const wchar_t* blank)
{
	// Load the shader and create the views
	loadComputeShader(cfile);
	createGPUViews();

	// Fill permutation table
	int permutation[256] =
	{
		151,160,137,91,90,15,
		131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
		190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
		88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
		77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
		102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
		135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
		5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
		223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
		129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
		251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
		49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
		138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
	};
	for (size_t i = 0; i < 512; i++)
	{
		permutationTable[i] = permutation[i % 256];	// Permutation doubled to avoid overflow
	}

	BufferCreationHelper::CreateStructuredBuffer(device, sizeof(int), 512, &permutationTable[0], &permutationBuffer);
	BufferCreationHelper::CreateBufferSRV(renderer, permutationBuffer, &permutationBufferSRV);
}
