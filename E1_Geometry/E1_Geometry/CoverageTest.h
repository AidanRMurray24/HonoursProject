#pragma once
#include "PerformanceTest.h"
#include "../DXFramework/FPCamera.h"

class CoverageTest : public PerformanceTest
{
public:
	CoverageTest(float* _coverage, FPCamera* _cam, class CloudContainer* _container);
	~CoverageTest();

	void StartTest();
	void CancelTest();
	bool UpdateEntries(float averageComputeTime);
	void SaveToFile();

private:
	std::vector<float> coverageAmounts;
	float* coverage;
	FPCamera* camera;
	class CloudContainer* container;
	float coverageIncrementAmount;
	float originalVal;
};

