#include "BaseEmitterSpawnProperty.h"
#include "MacroUtilities.h"
#include "BufferMacroUtilities.h"

#include <limits>

using namespace std;
using namespace D3D11;
using namespace DirectX;
using namespace ImGui;

BaseEmitterSpawnProperty::BaseEmitterSpawnProperty()
{
	AutoZeroMemory(m_emitterSpawnPropertyCPU);
	m_emitterSpawnPropertyCPU.initialParticleCount = 0;
	m_emitterSpawnPropertyCPU.initialParticleLife = 1.f;
}

void BaseEmitterSpawnProperty::SetShapedVectorProperty(const SShapedVectorProperty& shapedVectorSelector)
{
	m_emitterSpawnPropertyCPU.shapedVectorSelector = shapedVectorSelector;
	m_isEmitterSpawnPropertyChanged = true;
}

void BaseEmitterSpawnProperty::SetInitialParticleCount(UINT initialParticleCount)
{
	m_emitterSpawnPropertyCPU.initialParticleCount = initialParticleCount;
	m_isEmitterSpawnPropertyChanged = true;
}

void BaseEmitterSpawnProperty::SetInitialParticleLife(float initialParticleLife)
{
	m_emitterSpawnPropertyCPU.initialParticleLife = initialParticleLife;
	m_isEmitterSpawnPropertyChanged = true;
}

void BaseEmitterSpawnProperty::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_emitterSpawnPropertyGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_emitterSpawnPropertyCPU));
	m_emitterSpawnPropertyGPU->InitializeBuffer(device);
}

void BaseEmitterSpawnProperty::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	if (m_isEmitterSpawnPropertyChanged)
	{
		m_emitterSpawnPropertyGPU->Stage(deviceContext);
		m_emitterSpawnPropertyGPU->Upload(deviceContext);
	}
}

void BaseEmitterSpawnProperty::DrawPropertyUI()
{
	static bool isImmortal = false;
	static EShapedVector shapedVector = EShapedVector::None;

	if (!ImGui::CollapsingHeader("이미터 생성 프로퍼티"))
		return;

	if (DragInt("초기 파티클 개수", (int*)&m_emitterSpawnPropertyCPU.initialParticleCount, 1.f, 0, 100000))
	{
		m_isEmitterSpawnPropertyChanged = true;
	}

	BeginDisabled(isImmortal);
	{
		if (DragFloat("초기 파티클 생명", &m_emitterSpawnPropertyCPU.initialParticleLife, 0.1f, 0.f, 1000.f))
		{
			m_isEmitterSpawnPropertyChanged = true;
		}
	}
	EndDisabled();
	SameLine();
	Checkbox("Immortal 설정", &isImmortal);

	ShapedVectorSelector::SelectEnums("위치 초기 위치", ShapedVectorSelector::GShapedVectorStringMaps, shapedVector);
	if (ShapedVectorSelector::SetShapedVectorProperty(shapedVector, m_emitterSpawnPropertyCPU.shapedVectorSelector))
	{
		m_isEmitterSpawnPropertyChanged = true;
	}
}

