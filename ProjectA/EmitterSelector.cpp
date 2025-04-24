#include "EmitterSelector.h"

#include "ParticleEmitter.h"
#include "EmitterStaticData.h"

#include "BaseEmitterSpawnProperty.h"
#include "BaseEmitterUpdateProperty.h"
#include "BaseParticleSpawnProperty.h"
#include "BaseParticleUpdateProperty.h"


using namespace std;
using namespace ImGui;
using namespace DirectX;

static unordered_map<EEmitterType, string> GEmitterStringMaps
{
	{ EEmitterType::ParticleEmitter, "��ƼŬ �̹���" },
	{ EEmitterType::RibbonEmitter, "���� �̹���" },
	{ EEmitterType::SpriteEmitter, "��������Ʈ �̹���" },
	{ EEmitterType::MeshEmitter, "�Ž� �̹���" }
};

CEmitterSelector::CEmitterSelector(const string& selectorName)
	: CBaseSelector(selectorName, GEmitterStringMaps)
{
}

void CEmitterSelector::CreateEmitter(EEmitterType emitterType, unique_ptr<AEmitter>& emitter)
{
	switch (emitterType)
	{
	case EEmitterType::ParticleEmitter:
	{
		CreateParticleEmitter(emitter);
		break;
	}
	case EEmitterType::RibbonEmitter:
		break;
	case EEmitterType::SpriteEmitter:
		break;
	case EEmitterType::MeshEmitter:
		break;
	}

	if (emitter) emitter->CreateProperty();
}

void CEmitterSelector::CreateParticleEmitter(unique_ptr<AEmitter>& emitter)
{
	XMVECTOR position = XMVectorZero();
	XMVECTOR angle = XMVectorZero();
	UINT particleEmitterID = EmitterStaticData::IssueAvailableEmitterID();
	unique_ptr<ParticleEmitter> particleEmitter = make_unique<ParticleEmitter>(
		static_cast<UINT>(EEmitterType::ParticleEmitter),
		particleEmitterID,
		position,
		angle
	);
	emitter = std::move(particleEmitter);
}
