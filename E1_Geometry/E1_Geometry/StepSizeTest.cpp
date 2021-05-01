#include "StepSizeTest.h"
#include <fstream>
#include "CloudContainer.h"

StepSizeTest::StepSizeTest(float* _stepSize, FPCamera* _cam, CloudContainer* _container) : PerformanceTest()
{
	stepSize = _stepSize;
	camera = _cam;
	container = _container;
	numTimesToRecord = 10;
	maxStepSize = 10.0f;
	fileName = "StepSizeTest.csv";
	incrementSize = maxStepSize / numTimesToRecord;
}

StepSizeTest::~StepSizeTest()
{
}

void StepSizeTest::StartTest()
{
	// Clear the lists
	stepSizes.clear();
	recordedTimes.clear();

	// Set the camera's position and look at
	XMFLOAT3 containerPos = container->GetPosition();
	camera->setPosition(containerPos.x, containerPos.y, containerPos.z);
	camera->setRotation(90, 0, 0);

	*stepSize = 0;
}

bool StepSizeTest::UpdateEntries(float averageComputeTime)
{
	// Record the current values on the lists
	stepSizes.push_back(*stepSize);
	recordedTimes.push_back(averageComputeTime);

	// If the distance has reached the max distance then the testing has finished
	if (*stepSize >= maxStepSize)
	{
		*stepSize = maxStepSize;
		SaveToFile();
		return true;
	}

	// Calculate the new step size
	*stepSize += incrementSize;

	return false;
}

void StepSizeTest::SaveToFile()
{
	// Get the file path
	std::string filePath = folderPath.append(fileName);

	// Create the file at the file path's location
	std::ofstream outputFile(filePath);

	// Add coloumn headings
	outputFile << "Step Sizes, Compute Time\n";

	// Add data
	for (size_t i = 0; i < stepSizes.size(); i++)
		outputFile << stepSizes.at(i) << "," << recordedTimes.at(i) << std::endl;

	// Close the file
	outputFile.close();

	// Clear the lists
	stepSizes.clear();
	recordedTimes.clear();
}
