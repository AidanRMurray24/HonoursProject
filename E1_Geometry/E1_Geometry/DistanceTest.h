#pragma once
#include "PerformanceTest.h"
#include "../DXFramework/FPCamera.h"

class DistanceTest :public PerformanceTest
{
public:
	DistanceTest(FPCamera* _cam, class CloudContainer* _container);
	~DistanceTest();

	void StartTest();
	bool UpdateEntries(float averageComputeTime);
	void SaveToFile();

private:
	FPCamera* camera;
	class CloudContainer* container;
	float currentCameraDistance;
	std::vector<float> distances;
	float maxDistance;
	float distanceIncrement;
};

