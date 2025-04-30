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
		const std::function<void(UINT, UINT, bool, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterSelectedHandler,
		const std::function<void(UINT, UINT, bool, float, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterUpdatedHandler
	);
	~ParticleEmitter() override = default;

public:
	virtual void CreateProperty() override;
};

