#pragma once
#include "BaseSelector.h"
#include <memory>

class AEmitter;
class CBaseEmitterSpawnProperty;
class CBaseEmitterUpdateProperty;
class CBaseParticleSpawnProperty;
class CBaseParticleUpdateProperty;

enum class EEmitterType
{
	ParticleEmitter,
	RibbonEmitter,
	SpriteEmitter,
	MeshEmitter
};

class CEmitterSelector : public CBaseSelector<EEmitterType>
{
public:
	CEmitterSelector(const std::string& selectorName);
	~CEmitterSelector() override = default;

public:
	bool CreateEmitter(EEmitterType emitterType, std::unique_ptr<AEmitter>& emitter);

protected:
	bool CreateParticleEmitter(std::unique_ptr<AEmitter>& emitter);
	void InitializeParticleEmitterArgs(
		DirectX::XMVECTOR& position, DirectX::XMVECTOR& angle,
		uint32_t& particleEmitterID, std::unique_ptr<AEmitter/*ParticleEmitter*/>& particleEmitter,
		std::unique_ptr<CBaseEmitterSpawnProperty>& baseEmitterSpawnProperty,
		std::unique_ptr<CBaseEmitterUpdateProperty>& baseEmitterUpdateProperty,
		std::unique_ptr<CBaseParticleSpawnProperty>& baseParticleSpawnProperty,
		std::unique_ptr<CBaseParticleUpdateProperty>& baseParticleUpdateProperty
	);
};

