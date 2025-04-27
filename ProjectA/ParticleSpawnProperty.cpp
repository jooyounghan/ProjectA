#include "ParticleSpawnProperty.h"
#include "MacroUtilities.h"

using namespace std;

ParticleSpawnProperty::ParticleSpawnProperty(
	uint32_t maxEmitterCount,
	const function<void(const SParticleInterpInformation&)>& particleInterpInformationChangedHandler
)
	: 
	ARuntimeSpawnProperty(maxEmitterCount),
	m_onParticleInterpInformationChanged(particleInterpInformationChangedHandler)
{
	AutoZeroMemory(m_particleInterpInformation);
}

void ParticleSpawnProperty::OnInterpolateInformationChagned()
{
	m_particleInterpInformation.maxLife = m_baseParticleSpawnPropertyCPU.maxLife;
	m_particleInterpInformation.colorInterpolaterID = m_colorInterpolater->GetInterpolaterID();
	m_particleInterpInformation.colorInterpolaterDegree = m_colorInterpolater->GetCoefficientCount();
	m_onParticleInterpInformationChanged(m_particleInterpInformation);
}
