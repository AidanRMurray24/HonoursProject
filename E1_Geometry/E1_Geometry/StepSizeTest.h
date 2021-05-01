#pragma once
#include "PerformanceTest.h"
#include "../DXFramework/FPCamera.h"

class StepSizeTest : public PerformanceTest
{
public:
	StepSizeTest(float* _stepSize, FPCamera* _cam, class CloudContainer* _container);
	~StepSizeTest();

	void StartTest();
	bool UpdateEntries(float averageComputeTime);
	void SaveToFile();

private:
	float* stepSize;
	FPCamera* camera;
	class CloudContainer* container;
	float maxStepSize;
	float incrementSize;
	std::vector<float> stepSizes;
};

