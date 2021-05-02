#include "DistanceTest.h"
#include "CloudContainer.h"
#include <fstream>

DistanceTest::DistanceTest(FPCamera* _cam, CloudContainer* _container) : PerformanceTest()
{
	camera = _cam;
	container = _container;
	currentCameraDistance = 0;
	numTimesToRecord = 20;
	maxDistance = 100;
	fileName = "DistanceTest.csv";
	distanceIncrement = maxDistance / numTimesToRecord;
	estimatedTimeToComplete = 10 * numTimesToRecord * 0.4f;
}

DistanceTest::~DistanceTest()
{
}

void DistanceTest::StartTest()
{
	// Clear the lists
	recordedTimes.clear();
	distances.clear();

	// Set the camera's position and look at
	XMFLOAT3 containerPos = container->GetPosition();
	camera->setPosition(containerPos.x, containerPos.y, containerPos.z);
	camera->setRotation(-90, 0, 0);

	currentCameraDistance = 0;
}

bool DistanceTest::UpdateEntries(float averageComputeTime)
{
	// Record the current values on the lists
	distances.push_back(currentCameraDistance);
	recordedTimes.push_back(averageComputeTime);

	// If the distance has reached the max distance then the testing has finished
	if (currentCameraDistance >= maxDistance)
	{
		currentCameraDistance = maxDistance;
		SaveToFile();
		return true;
	}

	// Calculate the new distance value
	currentCameraDistance += distanceIncrement;

	// Update the camera's position
	XMFLOAT3 containerPos = container->GetPosition();
	camera->setPosition(containerPos.x, containerPos.y - currentCameraDistance, containerPos.z);

	return false;
}

void DistanceTest::SaveToFile()
{
	// Get the file path
	std::string filePath = folderPath + fileName;

	// Create the file at the file path's location
	std::ofstream outputFile(filePath);

	// Add coloumn headings
	outputFile << "Distance, Compute Time\n";

	// Add data
	for (size_t i = 0; i < distances.size(); i++)
		outputFile << distances.at(i) << "," << recordedTimes.at(i) << std::endl;

	// Close the file
	outputFile.close();

	// Clear the lists
	distances.clear();
	recordedTimes.clear();
}
