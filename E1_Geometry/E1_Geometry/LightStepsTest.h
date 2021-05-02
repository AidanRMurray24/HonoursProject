#pragma once
#include "PerformanceTest.h"
#include "../DXFramework/FPCamera.h"

class LightStepsTest : public PerformanceTest
{
public:
	LightStepsTest(int* _lightSteps, FPCamera* _cam, class CloudContainer* _container);
	~LightStepsTest();

	void StartTest();
	void CancelTest();
	bool UpdateEntries(float averageComputeTime);
	void SaveToFile();

private:
	int* lightSteps;
	FPCamera* camera;
	class CloudContainer* container;
	std::vector<int> recordedLightSteps;
	int maxLightSteps;
	int incrementAmount;
	int originalVal;
};

