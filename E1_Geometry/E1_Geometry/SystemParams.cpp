#include "SystemParams.h"

SystemParams::SystemParams()
{
	renderer = nullptr;
}

void SystemParams::CleanUp()
{
	assets.CleanUp();
}
