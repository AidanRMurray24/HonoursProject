#include "LightStepsTest.h"
#include "CloudContainer.h"

LightStepsTest::LightStepsTest(int* _lightSteps, FPCamera* _cam, CloudContainer* _container) : PerformanceTest()
{
	lightSteps = _lightSteps;
	camera = _cam;
	container = _container;
	numTimesToRecord = 20;
	maxLightSteps = 20;
	incrementAmount = maxLightSteps / numTimesToRecord;
	fileName = "LightStepsTest.csv";
	estimatedTimeToComplete = 10 * numTimesToRecord * 0.4f;
	originalVal = *lightSteps;
}

LightStepsTest::~LightStepsTest()
{
}

void LightStepsTest::StartTest()
{
	// Clear the lists
	recordedTimes.clear();
	recordedLightSteps.clear();

	// Set the camera's position and look at
	XMFLOAT3 containerPos = container->GetPosition();
	camera->setPosition(containerPos.x, containerPos.y - 100, containerPos.z);
	camera->setRotation(-90, 0, 0);

	originalVal = *lightSteps;
	*lightSteps = 0;
}

void LightStepsTest::CancelTest()
{
	*lightSteps = originalVal;
}

bool LightStepsTest::UpdateEntries(float averageComputeTime)
{
	// Record the current values on the lists
	recordedLightSteps.push_back(*lightSteps);
	recordedTimes.push_back(averageComputeTime);

	// If the distance has reached the max distance then the testing has finished
	if (*lightSteps >= maxLightSteps)
	{
		*lightSteps = originalVal;
		SaveToFile();
		return true;
	}

	// Calculate the new step size
	*lightSteps += incrementAmount;

	return false;
}

void LightStepsTest::SaveToFile()
{
	// Get the file path
	std::string filePath = folderPath + fileName;

	// Create the file at the file path's location
	std::ofstream outputFile(filePath);

	// Add coloumn headings
	outputFile << "Light Steps, Compute Time\n";

	// Add data
	for (size_t i = 0; i < recordedLightSteps.size(); i++)
		outputFile << recordedLightSteps.at(i) << "," << recordedTimes.at(i) << std::endl;

	// Close the file
	outputFile.close();

	// Clear the lists
	recordedLightSteps.clear();
	recordedTimes.clear();
}
