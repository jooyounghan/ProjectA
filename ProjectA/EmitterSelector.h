#pragma once
#include "BaseSelector.h"
#include <memory>

class AEmitter;
class BaseEmitterSpawnProperty;
class BaseEmitterUpdateProperty;
class BaseParticleSpawnProperty;
class BaseParticleUpdateProperty;

enum class EEmitterType
{
	ParticleEmitter,
	RibbonEmitter,
	SpriteEmitter,
	MeshEmitter
};

class EmitterSelector : public BaseSelector<EEmitterType>
{
public:
	EmitterSelector(const std::string& selectorName);
	~EmitterSelector() override = default;

public:
	bool CreateEmitter(EEmitterType emitterType, std::unique_ptr<AEmitter>& emitter);

protected:
	bool CreateParticleEmitter(std::unique_ptr<AEmitter>& emitter);
	void InitializeParticleEmitterArgs(
		DirectX::XMVECTOR& position, DirectX::XMVECTOR& angle,
		uint32_t& particleEmitterID, std::unique_ptr<AEmitter/*ParticleEmitter*/>& particleEmitter,
		std::unique_ptr<BaseEmitterSpawnProperty>& baseEmitterSpawnProperty,
		std::unique_ptr<BaseEmitterUpdateProperty>& baseEmitterUpdateProperty,
		std::unique_ptr<BaseParticleSpawnProperty>& baseParticleSpawnProperty,
		std::unique_ptr<BaseParticleUpdateProperty>& baseParticleUpdateProperty
	);
};

