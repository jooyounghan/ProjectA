#pragma once

enum class EEmitterType : size_t
{
	ParticleEmitter,
	SpriteEmitter,
	RibbonEmitter,
	MeshEmitter,
	EmitterTypeCount
};

constexpr size_t EmitterTypeCount = static_cast<size_t>(EEmitterType::EmitterTypeCount);