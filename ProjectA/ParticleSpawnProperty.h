#pragma once
#include "ARuntimeSpawnProperty.h"
#include "InterpInformation.h"

#include <functional>

class ParticleSpawnProperty : public ARuntimeSpawnProperty
{
public:
	ParticleSpawnProperty(
		uint32_t maxEmitterCount,
		const std::function<void(const SParticleInterpInformation&)>& particleInterpInformationChangedHandler
	);
	~ParticleSpawnProperty() override = default;

protected:
	std::function<void(const SParticleInterpInformation&)> m_onParticleInterpInformationChanged;

protected:
	SParticleInterpInformation m_particleInterpInformation;

protected:
	virtual void OnInterpolateInformationChagned() override;
};

