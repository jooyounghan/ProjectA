#include "ParticleEmitter.h"
#include "BaseEmitterSpawnProperty.h"
#include "BaseEmitterUpdateProperty.h"
#include "BaseParticleSpawnProperty.h"
#include "BaseParticleUpdateProperty.h"

using namespace std;

ParticleEmitter::ParticleEmitter(
	UINT emitterType, 
	UINT emitterID, 
	const DirectX::XMVECTOR& position, 
	const DirectX::XMVECTOR& angle
)
	: AEmitter(emitterType, emitterID, position, angle)
{

}

void ParticleEmitter::CreateProperty()
{
	m_emitterSpawnProperty = make_unique<CBaseEmitterSpawnProperty>();
	m_emitterUpdateProperty = make_unique<CBaseEmitterUpdateProperty>();
	m_particleSpawnProperty = make_unique<CBaseParticleSpawnProperty>(
		[this](float life) { SetInterpolaterLifeInformation(life); },
		[this](UINT interpolaterID, UINT interpolaterDegree) { SetColorInterpolaterInformation(interpolaterID, interpolaterDegree); }
	);
	m_particleUpdateProperty = make_unique<CBaseParticleUpdateProperty>(
		[this](const SEmitterForceProperty forceProperty) { SetEmitterForceProperty(forceProperty); }
	);
}
