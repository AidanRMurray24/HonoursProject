#include "GPUTimer.h"

GPUTimer::GPUTimer(ID3D11Device* _device, ID3D11DeviceContext* dc)
{
	device = _device;
	deviceContext = dc;

	// Create start and end timestamp queries
	D3D11_QUERY_DESC timeStampDesc;
	timeStampDesc.Query = D3D11_QUERY_TIMESTAMP;
	timeStampDesc.MiscFlags = 0;
	device->CreateQuery(&timeStampDesc, &startQuery);
	device->CreateQuery(&timeStampDesc, &endQuery);

	// Create the disjoint query
	D3D11_QUERY_DESC disjointDesc;
	disjointDesc.Query = D3D11_QUERY_TIMESTAMP_DISJOINT;
	disjointDesc.MiscFlags = 0;
	device->CreateQuery(&disjointDesc, &disjointQuery);
}

GPUTimer::~GPUTimer()
{
}

void GPUTimer::StartTimer()
{
	deviceContext->Begin(disjointQuery);
	deviceContext->End(startQuery);
}

void GPUTimer::StopTimer()
{
	deviceContext->End(endQuery);
	deviceContext->End(disjointQuery);
}

double GPUTimer::GetTimeTaken()
{
	// Get back the data from the queries
	UINT64 startData = 0;
	deviceContext->GetData(startQuery, &startData, sizeof(UINT64), 0);
	UINT64 endData = 0;
	deviceContext->GetData(endQuery, &endData, sizeof(UINT64), 0);
	D3D11_QUERY_DATA_TIMESTAMP_DISJOINT disjointData;
	deviceContext->GetData(disjointQuery, &disjointData, sizeof(D3D11_QUERY_DATA_TIMESTAMP_DISJOINT), 0);

	// Subtract the end timestamp value from the start timestamp value to get the number of ticks
	UINT64 elapsedFrames = endData - startData;

	// Divide the number of frames by the frequency to get the time taken in milliseconds
	return ((double)elapsedFrames / (double)disjointData.Frequency) * 1000.0f;
}
