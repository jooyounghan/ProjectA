#include "EmitterSelector.h"
#include "AEmitter.h"
#include "BaseEmitterSpawnProperty.h"
#include "BaseEmitterUpdateProperty.h"
#include "BaseParticleSpawnProperty.h"
#include "BaseParticleUpdateProperty.h"


using namespace std;
using namespace ImGui;
using namespace DirectX;

unordered_map<EEmitterType, string> EmitterSelector::GEmitterStringMaps
{
	{ EEmitterType::ParticleEmitter, "파티클 이미터" },
	{ EEmitterType::RibbonEmitter, "리본 이미터" },
	{ EEmitterType::SpriteEmitter, "스프라이트 이미터" },
	{ EEmitterType::MeshEmitter, "매시 이미터" }
};

bool EmitterSelector::CreateEmitter(EEmitterType emitterType, unique_ptr<AEmitter>& emitter)
{
	bool result = false;
	switch (emitterType)
	{
	case EEmitterType::ParticleEmitter:
	{
		result = CreateParticleEmitter(emitter);
		break;
	}
	case EEmitterType::RibbonEmitter:
		break;
	case EEmitterType::SpriteEmitter:
		break;
	case EEmitterType::MeshEmitter:
		break;
	}

	return result;
}

bool EmitterSelector::CreateParticleEmitter(unique_ptr<AEmitter>& emitter)
{
	static XMVECTOR position = XMVectorZero();
	static XMVECTOR angle = XMVectorZero();
	static UINT particleEmitterID = AEmitter::IssueAvailableEmitterID();
	static std::unique_ptr<AEmitter/*ParticleEmitter*/> particleEmitter = make_unique<AEmitter>(
		static_cast<UINT>(EEmitterType::ParticleEmitter), particleEmitterID,
		AEmitter::GIsEmitterWorldPositionChanged,
		AEmitter::GEmitterWorldTransformCPU[particleEmitterID],
		AEmitter::GIsEmitterForceChanged,
		AEmitter::GEmitterForcePropertyCPU[particleEmitterID],
		position,
		angle
	);
	static std::unique_ptr<BaseEmitterSpawnProperty> baseEmitterSpawnProperty = make_unique<BaseEmitterSpawnProperty>();
	static std::unique_ptr<BaseEmitterUpdateProperty> baseEmitterUpdateProperty = make_unique<BaseEmitterUpdateProperty>(particleEmitter->GetCurrnetEmitter(), particleEmitter->GetLoopTime());
	static std::unique_ptr<BaseParticleSpawnProperty> baseParticleSpawnProperty = make_unique<BaseParticleSpawnProperty>(particleEmitter->GetCurrnetEmitter(), particleEmitter->GetLoopTime());
	static std::unique_ptr<BaseParticleUpdateProperty> baseParticleUpdateProperty = make_unique<BaseParticleUpdateProperty>(particleEmitter->GetIsEmitterForceChanged(), particleEmitter->GetEmitterForce());

	baseEmitterSpawnProperty->DrawPropertyUI();
	baseEmitterUpdateProperty->DrawPropertyUI();
	baseParticleSpawnProperty->DrawPropertyUI();
	baseParticleUpdateProperty->DrawPropertyUI();

	if (Button("설정"))
	{
		particleEmitter->InjectAEmitterSpawnProperty(baseEmitterSpawnProperty);
		particleEmitter->InjectAEmitterUpdateProperty(baseEmitterUpdateProperty);
		particleEmitter->InjectAParticleSpawnProperty(baseParticleSpawnProperty);
		particleEmitter->InjectAParticleUpdateProperty(baseParticleUpdateProperty);
		emitter = std::move(particleEmitter);
		InitializeParticleEmitterArgs(position, angle, particleEmitterID, particleEmitter,
			baseEmitterSpawnProperty, baseEmitterUpdateProperty,
			baseParticleSpawnProperty, baseParticleUpdateProperty
		);
		return true;
	}
	return false;
}

void EmitterSelector::InitializeParticleEmitterArgs(
	XMVECTOR& position, XMVECTOR& angle, uint32_t& particleEmitterID,
	unique_ptr<AEmitter>& particleEmitter,
	unique_ptr<BaseEmitterSpawnProperty>& baseEmitterSpawnProperty, 
	unique_ptr<BaseEmitterUpdateProperty>& baseEmitterUpdateProperty, 
	unique_ptr<BaseParticleSpawnProperty>& baseParticleSpawnProperty, 
	unique_ptr<BaseParticleUpdateProperty>& baseParticleUpdateProperty
)
{
	position = XMVectorZero();
	angle = XMVectorZero();
	particleEmitterID = AEmitter::IssueAvailableEmitterID();
	particleEmitter = make_unique<AEmitter>(
		static_cast<UINT>(EEmitterType::ParticleEmitter), particleEmitterID,
		AEmitter::GIsEmitterWorldPositionChanged,
		AEmitter::GEmitterWorldTransformCPU[particleEmitterID],
		AEmitter::GIsEmitterForceChanged,
		AEmitter::GEmitterForcePropertyCPU[particleEmitterID],
		position,
		angle
	);
	baseEmitterSpawnProperty = make_unique<BaseEmitterSpawnProperty>();
	baseEmitterUpdateProperty = make_unique<BaseEmitterUpdateProperty>(particleEmitter->GetCurrnetEmitter(), particleEmitter->GetLoopTime());
	baseParticleSpawnProperty = make_unique<BaseParticleSpawnProperty>(particleEmitter->GetCurrnetEmitter(), particleEmitter->GetLoopTime());
	baseParticleUpdateProperty = make_unique<BaseParticleUpdateProperty>(particleEmitter->GetIsEmitterForceChanged(), particleEmitter->GetEmitterForce());
}
