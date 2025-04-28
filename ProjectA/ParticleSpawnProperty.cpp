#include "ParticleSpawnProperty.h"
#include "MacroUtilities.h"

using namespace std;

ParticleSpawnProperty::ParticleSpawnProperty(
	const std::function<void(EInterpolationMethod, bool)>& gpuColorInterpolaterSelectHandler,
	const std::function<void(EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterUpdatedHandler,
	const function<void(float, UINT)>& particleInterpInformChangedHandler
)
	: 
	ARuntimeSpawnProperty(gpuColorInterpolaterSelectHandler, gpuColorInterpolaterUpdatedHandler),
	m_onParticleInterpInforChanged(particleInterpInformChangedHandler)
{

}
