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
	{ EEmitterType::ParticleEmitter, "��ƼŬ �̹���" },
	{ EEmitterType::SpriteEmitter, "��������Ʈ �̹���" },
	{ EEmitterType::RibbonEmitter, "���� �̹���" },
	{ EEmitterType::MeshEmitter, "�Ž� �̹���" }
};

CEmitterSelector::CEmitterSelector(const string& selectorName)
	: CBaseSelector(selectorName, GEmitterStringMaps)
{
}