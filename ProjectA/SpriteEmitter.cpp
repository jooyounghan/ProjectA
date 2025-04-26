#include "SpriteEmitter.h"
#include "InitialSpawnProperty.h"
#include "EmitterUpdateProperty.h"
#include "SpriteSpawnProperty.h"
#include "ForceUpdateProperty.h"

#include "EmitterTypeDefinition.h"

using namespace std;
using namespace DirectX;

SpriteEmitter::SpriteEmitter(
	UINT emitterID, 
	const DirectX::XMVECTOR& position, 
	const DirectX::XMVECTOR& angle, 
	const std::function<void(UINT, const DirectX::XMMATRIX&)>& worldTransformChangedHandler,
	const std::function<void(UINT, const SEmitterForceProperty&)>& forcePropertyChangedHandler, 
	const std::function<void(UINT, const SSpriteInterpInformation&)>& interpInformationChangedHandler
)
	: AEmitter(
		static_cast<UINT>(EEmitterType::SpriteEmitter),
		emitterID, position, angle,
		worldTransformChangedHandler,
		forcePropertyChangedHandler
	),
	m_onInterpInformationChanged(interpInformationChangedHandler)
{
}

void SpriteEmitter::CreateProperty()
{
	m_initialSpawnProperty = make_unique<CInitialSpawnProperty>();
	m_emitterUpdateProperty = make_unique<CEmitterUpdateProperty>();
	m_runtimeSpawnProperty = make_unique<SpriteSpawnProperty>(
		[this](const SSpriteInterpInformation& interpInformation) { m_onInterpInformationChanged(GetEmitterID(), interpInformation); }
	);
	m_forceUpdateProperty = make_unique<ForceUpdateProperty>(
		[this](const SEmitterForceProperty& forceProperty) { m_onForcePropertyChanged(GetEmitterID(), forceProperty); }
	);

}
