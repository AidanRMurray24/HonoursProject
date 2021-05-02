#include "CoverageTest.h"
#include <fstream>
#include "CloudContainer.h"

CoverageTest::CoverageTest(float* _coverage, FPCamera* _cam, CloudContainer* _container) : PerformanceTest()
{
	coverage = _coverage;
	camera = _cam;
	container = _container;
	fileName = "CoverageTest.csv";
	numTimesToRecord = 20;
	coverageIncrementAmount = 1.0f / numTimesToRecord;
	estimatedTimeToComplete = 10 * numTimesToRecord * 0.4f;
	originalVal = *coverage;
}

CoverageTest::~CoverageTest()
{
}

void CoverageTest::StartTest()
{
	// Clear the lists
	coverageAmounts.clear();
	recordedTimes.clear();

	// Set the camera's position and look at
	XMFLOAT3 containerPos = container->GetPosition();
	camera->setPosition(containerPos.x, containerPos.y - 100, containerPos.z);
	camera->setRotation(-90,0,0);

	// Set the coverage of the clouds to be 0 intially
	originalVal = *coverage;
	*coverage = 0;
}

void CoverageTest::CancelTest()
{
	*coverage = originalVal;
}

// Returns true if 
bool CoverageTest::UpdateEntries(float averageComputeTime)
{
	// Record the current values on the lists
	coverageAmounts.push_back(*coverage);
	recordedTimes.push_back(averageComputeTime);

	// If the coverage has reached 1 then the testing has finished
	if (*coverage >= 1)
	{
		*coverage = originalVal;
		SaveToFile();
		return true;
	}

	// Calculate the new coverage value
	*coverage += coverageIncrementAmount;

	return false;
}

void CoverageTest::SaveToFile()
{
	// Get the file path
	std::string filePath = folderPath + fileName;

	// Create the file at the file path's location
	std::ofstream outputFile(filePath);

	// Add coloumn headings
	outputFile << "Coverage, Compute Time\n";

	// Add data
	for (size_t i = 0; i < coverageAmounts.size(); i++)
		outputFile << coverageAmounts.at(i) << "," << recordedTimes.at(i) << std::endl;

	// Close the file
	outputFile.close();

	// Clear the lists
	coverageAmounts.clear();
	recordedTimes.clear();
}
