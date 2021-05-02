#pragma once
#include "PerformanceTest.h"
#include "../DXFramework/FPCamera.h"

class ReprojectionTest : public PerformanceTest
{
public:
	ReprojectionTest(bool* _useTemporalReprojection, FPCamera* _cam, class CloudContainer* _container);
	~ReprojectionTest();

	void StartTest();
	void CancelTest();
	bool UpdateEntries(float averageComputeTime);
	void SaveToFile();

private:
	FPCamera* camera;
	class CloudContainer* container;
	bool* useTemporalReprojection;
	std::vector<bool> recordedReprojectionVals;
	bool originalVal;
};

