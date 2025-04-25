#pragma once

enum class EEmitterType : size_t
{
	ParticleEmitter,
	RibbonEmitter,
	SpriteEmitter,
	MeshEmitter,
	EmitterTypeCount
};

constexpr size_t EmitterTypeCount = static_cast<size_t>(EEmitterType::EmitterTypeCount);