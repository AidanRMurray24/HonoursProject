#pragma once
#include "../DXFramework/DXF.h"

class GPUTimer
{
public:
	GPUTimer(ID3D11Device* device, ID3D11DeviceContext* dc);
	~GPUTimer();

	void StartTimer();
	void StopTimer();
	double GetTimeTaken();

private:
	ID3D11Device* device;
	ID3D11DeviceContext* deviceContext;

	ID3D11Query* startQuery;
	ID3D11Query* endQuery;
	ID3D11Query* disjointQuery;
};

