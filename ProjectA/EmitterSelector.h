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
	static void CreateEmitter(EEmitterType emitterType, std::unique_ptr<AEmitter>& emitter);

protected:
	static void CreateParticleEmitter(std::unique_ptr<AEmitter>& emitter);
};

