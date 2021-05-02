#pragma once
#include <string>
#include <vector>

class PerformanceTest
{
public:
	PerformanceTest();
	~PerformanceTest();

	virtual void StartTest() {};
	virtual void CancelTest() {};
	virtual bool UpdateEntries(float averageComputeTime) { return true; };
	virtual void SaveToFile() {};

	// Getters
	inline int GetNumTimesToRecord() { return numTimesToRecord; };
	inline float GetEstimatedTimeToComplete() {return estimatedTimeToComplete;};

protected:

	int numTimesToRecord;
	std::string fileName;
	std::string folderPath;
	std::vector<float> recordedTimes;
	float estimatedTimeToComplete;
};

