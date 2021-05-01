#include "PerformanceTest.h"

PerformanceTest::PerformanceTest()
{
	estimatedTimeToComplete = 100.0f;
	numTimesToRecord = 0;
	fileName = "";
	folderPath = "Tests/";
}

PerformanceTest::~PerformanceTest()
{
}
