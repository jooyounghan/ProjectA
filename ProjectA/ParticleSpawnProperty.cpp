#include "ParticleSpawnProperty.h"
#include "MacroUtilities.h"

using namespace std;

ParticleSpawnProperty::ParticleSpawnProperty(
	const function<void(const SParticleInterpInformation&)>& particleInterpInformationChangedHandler
)
	: m_onParticleInterpInformationChanged(particleInterpInformationChangedHandler)
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
