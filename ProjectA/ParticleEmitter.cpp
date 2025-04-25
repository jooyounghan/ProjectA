#include "ParticleEmitter.h"
#include "InitialSpawnProperty.h"
#include "EmitterUpdateProperty.h"
#include "ParticleSpawnProperty.h"
#include "ForceUpdateProperty.h"

#include "EmitterTypeDefinition.h"

using namespace std;
using namespace DirectX;

ParticleEmitter::ParticleEmitter(
	UINT emitterID, 
	const XMVECTOR& position, 
	const XMVECTOR& angle,
	const function<void(UINT, const XMMATRIX&)>& worldTransformChangedHandler,
	const function<void(UINT, const SEmitterForceProperty&)>& forcePropertyChangedHandler,
	const function<void(UINT, const SParticleInterpInformation&)>& interpInformationChangedHandler
)
	: AEmitter(
		static_cast<UINT>(EEmitterType::ParticleEmitter),
		emitterID, position, angle,
		worldTransformChangedHandler,
		forcePropertyChangedHandler
	),
	m_onInterpInformationChanged(interpInformationChangedHandler)
{

}

void ParticleEmitter::CreateProperty()
{
	m_initialSpawnProperty = make_unique<CInitialSpawnProperty>();
	m_emitterUpdateProperty = make_unique<CEmitterUpdateProperty>();
	m_runtimeSpawnProperty = make_unique<ParticleSpawnProperty>(
		[this](const SParticleInterpInformation& interpInformation) { m_onInterpInformationChanged(GetEmitterID(), interpInformation); }
	);
	m_forceUpdateProperty = make_unique<ForceUpdateProperty>(
		[this](const SEmitterForceProperty& forceProperty) { m_onForcePropertyChanged(GetEmitterID(), forceProperty); }
	);
}

