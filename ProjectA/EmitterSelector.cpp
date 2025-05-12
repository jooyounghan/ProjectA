#include "EmitterSelector.h"

#include "ParticleEmitter.h"
#include "SpriteEmitter.h"

#include "InitialSpawnProperty.h"
#include "EmitterUpdateProperty.h"
#include "RuntimeSpawnProperty.h"
#include "ForceUpdateProperty.h"


using namespace std;
using namespace ImGui;
using namespace DirectX;

static unordered_map<EEmitterType, string> GEmitterStringMaps
{
	{ EEmitterType::ParticleEmitter, "파티클 이미터" },
	{ EEmitterType::SpriteEmitter, "스프라이트 이미터" },
	{ EEmitterType::RibbonEmitter, "리본 이미터" },
	{ EEmitterType::MeshEmitter, "매시 이미터" }
};

CEmitterSelector::CEmitterSelector(const string& selectorName)
	: CBaseSelector(selectorName, GEmitterStringMaps)
{
}