#include "BaseEmitterSpawnProperty.h"
#include "MacroUtilities.h"
#include "BufferMacroUtilities.h"

#include <limits>

using namespace std;
using namespace D3D11;
using namespace DirectX;
using namespace ImGui;

BaseEmitterSpawnProperty::BaseEmitterSpawnProperty()
	: m_isEmitterSpawnPropertyChanged(false),
	m_shapedVector(EShapedVector::Manual),
	m_isImmortal(false)
{
	AutoZeroMemory(m_emitterSpawnPropertyCPU);
	m_emitterSpawnPropertyCPU.initialParticleCount = 0;
	m_emitterSpawnPropertyCPU.initialParticleLife = 1.f;
	m_emitterSpawnPropertyCPU.color = XMVectorSet(1.f, 1.f, 1.f, 1.f);

	m_shapedPositionSelector = make_unique<ShapedVectorSelector>(
		"초기 위치 벡터", "반지름",
		m_emitterSpawnPropertyCPU.shapedPositionVectorProperty
	);

	sizeof(m_emitterSpawnPropertyCPU);
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
		m_isEmitterSpawnPropertyChanged = false;
	}
}

void BaseEmitterSpawnProperty::DrawPropertyUI()
{
	if (!ImGui::CollapsingHeader("이미터 생성 프로퍼티"))
		return;

	if (DragInt("초기 파티클 개수", (int*)&m_emitterSpawnPropertyCPU.initialParticleCount, 1.f, 0, 100000))
	{
		m_isEmitterSpawnPropertyChanged = true;
	}

	BeginDisabled(m_isImmortal);
	{
		if (DragFloat("초기 파티클 생명", &m_emitterSpawnPropertyCPU.initialParticleLife, 0.1f, 0.f, 1000.f, m_isImmortal ? "무한" : "%.1f"))
		{
			m_isEmitterSpawnPropertyChanged = true;
		}
	}
	EndDisabled();
	SameLine();
	if (Checkbox("Immortal 설정", &m_isImmortal))
	{
		m_emitterSpawnPropertyCPU.initialParticleLife = m_isImmortal ? numeric_limits<float>::max() : 1.f;
	}

	if (ColorPicker3("초기 파티클 색상", m_emitterSpawnPropertyCPU.color.m128_f32))
	{
		m_isEmitterSpawnPropertyChanged = true;
	}

	m_shapedPositionSelector->SelectEnums(m_shapedVector);
	if (m_shapedPositionSelector->SetShapedVectorProperty(m_shapedVector))
	{
		m_isEmitterSpawnPropertyChanged = true;
	}
}

