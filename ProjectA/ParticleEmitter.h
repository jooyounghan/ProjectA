#pragma once
#include "AEmitter.h"
#include "ParticleInterpInformation.h"

class ParticleEmitter : public AEmitter
{
public:
	ParticleEmitter(
		UINT emitterID,
		const DirectX::XMVECTOR& position,
		const DirectX::XMVECTOR& angle,
		const std::function<void(UINT, const DirectX::XMMATRIX&)>& worldTransformChangedHandler,
		const std::function<void(UINT, const SEmitterForceProperty&)>& forcePropertyChangedHandler,
		const std::function<void(UINT, const SParticleInterpInformation&)>& interpInformationChangedHandler
	);
	~ParticleEmitter() override = default;

protected:
	std::function<void(UINT, const SParticleInterpInformation&)> m_onInterpInformationChanged;

protected:
	virtual void CreateProperty() override;
};

