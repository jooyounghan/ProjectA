#include "EmitterSelector.h"

#include "AEmitter.h"
#include "EmitterStaticData.h"

#include "DynamicBuffer.h"
#include "BaseEmitterSpawnProperty.h"
#include "BaseEmitterUpdateProperty.h"
#include "BaseParticleSpawnProperty.h"
#include "BaseParticleUpdateProperty.h"


using namespace std;
using namespace ImGui;
using namespace DirectX;

static unordered_map<EEmitterType, string> GEmitterStringMaps
{
	{ EEmitterType::ParticleEmitter, "파티클 이미터" },
	{ EEmitterType::RibbonEmitter, "리본 이미터" },
	{ EEmitterType::SpriteEmitter, "스프라이트 이미터" },
	{ EEmitterType::MeshEmitter, "매시 이미터" }
};

CEmitterSelector::CEmitterSelector(const string& selectorName)
	: CBaseSelector(selectorName, GEmitterStringMaps)
{
}

bool CEmitterSelector::CreateEmitter(EEmitterType emitterType, unique_ptr<AEmitter>& emitter)
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

bool CEmitterSelector::CreateParticleEmitter(unique_ptr<AEmitter>& emitter)
{
	static XMVECTOR position = XMVectorZero();
	static XMVECTOR angle = XMVectorZero();
	static UINT particleEmitterID = EmitterStaticData::IssueAvailableEmitterID();
	static std::unique_ptr<AEmitter/*ParticleEmitter*/> particleEmitter = make_unique<AEmitter>(
		static_cast<UINT>(EEmitterType::ParticleEmitter), particleEmitterID,
		EmitterStaticData::GEmitterWorldTransformCPU[particleEmitterID],
		EmitterStaticData::GEmitterForcePropertyCPU[particleEmitterID],
		position,
		angle
	);

	AEmitter* currentEmitter = particleEmitter.get();

	static std::unique_ptr<CBaseEmitterSpawnProperty> baseEmitterSpawnProperty = make_unique<CBaseEmitterSpawnProperty>();
	static std::unique_ptr<CBaseEmitterUpdateProperty> baseEmitterUpdateProperty = make_unique<CBaseEmitterUpdateProperty>(particleEmitter->GetCurrnetEmitter(), particleEmitter->GetLoopTime());
	static std::unique_ptr<CBaseParticleSpawnProperty> baseParticleSpawnProperty = make_unique<CBaseParticleSpawnProperty>(
		[currentEmitter](UINT interpolaterID, UINT interpolaterDegree) { currentEmitter->SetColorInterpolaterProperty(interpolaterID, interpolaterDegree); }
	);
	static std::unique_ptr<CBaseParticleUpdateProperty> baseParticleUpdateProperty = make_unique<CBaseParticleUpdateProperty>(
		particleEmitterID,
		particleEmitter->GetEmitterForce(), 
		[](UINT forcePropertyIndex) { EmitterStaticData::AddChangedEmitterForceID(forcePropertyIndex); }
	);

	baseEmitterSpawnProperty->DrawPropertyUI();
	baseEmitterUpdateProperty->DrawPropertyUI();
	baseParticleSpawnProperty->DrawPropertyUI();
	baseParticleUpdateProperty->DrawPropertyUI();

	Separator();
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

void CEmitterSelector::InitializeParticleEmitterArgs(
	XMVECTOR& position, XMVECTOR& angle, uint32_t& particleEmitterID,
	unique_ptr<AEmitter>& particleEmitter,
	unique_ptr<CBaseEmitterSpawnProperty>& baseEmitterSpawnProperty, 
	unique_ptr<CBaseEmitterUpdateProperty>& baseEmitterUpdateProperty, 
	unique_ptr<CBaseParticleSpawnProperty>& baseParticleSpawnProperty, 
	unique_ptr<CBaseParticleUpdateProperty>& baseParticleUpdateProperty
)
{
	position = XMVectorZero();
	angle = XMVectorZero();
	particleEmitterID = EmitterStaticData::IssueAvailableEmitterID();
	particleEmitter = make_unique<AEmitter>(
		static_cast<UINT>(EEmitterType::ParticleEmitter), particleEmitterID,
		EmitterStaticData::GEmitterWorldTransformCPU[particleEmitterID],
		EmitterStaticData::GEmitterForcePropertyCPU[particleEmitterID],
		position,
		angle
	);

	AEmitter* emitter = particleEmitter.get();

	baseEmitterSpawnProperty = make_unique<CBaseEmitterSpawnProperty>();
	baseEmitterUpdateProperty = make_unique<CBaseEmitterUpdateProperty>(particleEmitter->GetCurrnetEmitter(), particleEmitter->GetLoopTime());
	baseParticleSpawnProperty = make_unique<CBaseParticleSpawnProperty>(
		[emitter](UINT interpolaterPropertyID, UINT interpolaterDegree) {emitter->SetColorInterpolaterProperty(interpolaterPropertyID, interpolaterDegree); }
	);
	baseParticleUpdateProperty = make_unique<CBaseParticleUpdateProperty>(
		particleEmitterID,
		particleEmitter->GetEmitterForce(),
		[](UINT forcePropertyIndex) { EmitterStaticData::AddChangedEmitterForceID(forcePropertyIndex); }
	);
}
