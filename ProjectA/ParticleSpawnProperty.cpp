#include "ParticleSpawnProperty.h"
#include "MacroUtilities.h"

using namespace std;

ParticleSpawnProperty::ParticleSpawnProperty(
	const function<void(bool, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterSelectedHandler,
	const function<void(bool, float, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterUpdatedHandler
)
	: 
	ARuntimeSpawnProperty(gpuColorInterpolaterSelectedHandler, gpuColorInterpolaterUpdatedHandler)
{

}
