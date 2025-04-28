#pragma once
#include "AEmitter.h"
#include "InterpInformation.h"

class ParticleEmitter : public AEmitter
{
public:
	ParticleEmitter(
		UINT emitterID,
		const DirectX::XMVECTOR& position,
		const DirectX::XMVECTOR& angle,
		const std::function<void(UINT, const DirectX::XMMATRIX&)>& worldTransformChangedHandler,
		const std::function<void(UINT, const SEmitterForceProperty&)>& forcePropertyChangedHandler,
		const std::function<void(UINT, EInterpolationMethod, bool)>& gpuColorInterpolaterSelectedHandler,
		const std::function<void(UINT, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterUpdatedHandler,
		const std::function<void(UINT, float, UINT, UINT)>& particleInterpInformChangedHandler
	);
	~ParticleEmitter() override = default;

protected:
	std::function<void(UINT, float, UINT, UINT)> m_onParticleInterpInforChanged;

protected:
	virtual void CreateProperty() override;
};

