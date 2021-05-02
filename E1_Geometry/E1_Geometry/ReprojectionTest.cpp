#include "ReprojectionTest.h"
#include "CloudContainer.h"

ReprojectionTest::ReprojectionTest(bool* _useTemporalReprojection, FPCamera* _cam, CloudContainer* _container) : PerformanceTest()
{
	useTemporalReprojection = _useTemporalReprojection;
	camera = _cam;
	container = _container;
	fileName = "ReprojectionTest.csv";
	numTimesToRecord = 2;
	estimatedTimeToComplete = 10.0f * numTimesToRecord * 0.4f;
	originalVal = *useTemporalReprojection;
}

ReprojectionTest::~ReprojectionTest()
{
}

void ReprojectionTest::StartTest()
{
	// Clear the lists
	recordedTimes.clear();
	recordedReprojectionVals.clear();

	// Set the camera's position and look at
	XMFLOAT3 containerPos = container->GetPosition();
	camera->setPosition(containerPos.x, containerPos.y - 100, containerPos.z);
	camera->setRotation(-90, 0, 0);

	originalVal = *useTemporalReprojection;
	*useTemporalReprojection = false;
}

void ReprojectionTest::CancelTest()
{
	*useTemporalReprojection = originalVal;
}

bool ReprojectionTest::UpdateEntries(float averageComputeTime)
{
	// Record the current values on the lists
	recordedReprojectionVals.push_back(*useTemporalReprojection);
	recordedTimes.push_back(averageComputeTime);

	// If temporal reprojection is on and has already been recorded then return true
	if (*useTemporalReprojection)
	{
		*useTemporalReprojection = originalVal;
		SaveToFile();
		return true;
	}

	*useTemporalReprojection = true;

	return false;
}

void ReprojectionTest::SaveToFile()
{
	// Get the file path
	std::string filePath = folderPath + fileName;

	// Create the file at the file path's location
	std::ofstream outputFile(filePath);

	// Add coloumn headings
	outputFile << "Reprojection Enabled, Compute Time\n";

	// Add data
	for (size_t i = 0; i < recordedReprojectionVals.size(); i++)
		outputFile << recordedReprojectionVals.at(i) << "," << recordedTimes.at(i) << std::endl;

	// Close the file
	outputFile.close();

	// Clear the lists
	recordedReprojectionVals.clear();
	recordedTimes.clear();
}
