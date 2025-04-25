#include "ParticleSpawnProperty.h"

#include "Interpolater.h"
#include "ShapedVectorSelector.h"
#include "InterpolaterSelector.h"
#include "ControlPointGridView.h"

using namespace std;

ParticleSpawnProperty::ParticleSpawnProperty(
	const function<void(const SParticleInterpInformation&)>& particleInterpInformationChangedHandler
)
	: m_onParticleInterpInformationChanged(particleInterpInformationChangedHandler)
{
}

void ParticleSpawnProperty::OnInterpolateInformationChagned()
{
	m_particleInterpInformation.maxLife = m_baseParticleSpawnPropertyCPU.maxLife;
	m_particleInterpInformation.colorInterpolaterID = m_colorInterpolater->GetInterpolaterID();
	m_particleInterpInformation.colorInterpolaterDegree = m_colorInterpolater->GetCoefficientCount();
	m_onParticleInterpInformationChanged(m_particleInterpInformation);
}
