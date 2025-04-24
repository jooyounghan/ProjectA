#pragma once
#include "AEmitter.h"

class ParticleEmitter : public AEmitter
{
public:
	ParticleEmitter(
		UINT emitterType,
		UINT emitterID,
		const DirectX::XMVECTOR& position,
		const DirectX::XMVECTOR& angle
	);
	~ParticleEmitter() override = default;

public:
	virtual void CreateProperty() override;
};

