#include "BaseEmitterSpawnProperty.h"
#include "MacroUtilities.h"
#include "BufferMacroUtilities.h"

#include "DynamicBuffer.h"
#include "ShapedVectorSelector.h"

#include <limits>

using namespace std;
using namespace D3D11;
using namespace DirectX;
using namespace ImGui;

CBaseEmitterSpawnProperty::CBaseEmitterSpawnProperty()
	: m_isEmitterSpawnPropertyChanged(false),
	m_positionShapedVector(EShapedVector::Sphere),
	m_speedShapedVector(EShapedVector::None),
	m_isImmortal(false)
{
	AutoZeroMemory(m_emitterSpawnPropertyCPU);
	m_emitterSpawnPropertyCPU.initialParticleCount = 0;
	m_emitterSpawnPropertyCPU.initialParticleLife = 1.f;

	m_positionShapedVectorSelector = make_unique<CShapedVectorSelector>(
		"초기 위치 벡터", "초기 반지름",
		m_emitterSpawnPropertyCPU.shapedPositionVectorProperty
	);

	m_speedShapedVectorSelector = make_unique<CShapedVectorSelector>(
		"초기 속도 벡터", "초기 속도",
		m_emitterSpawnPropertyCPU.shapedSpeedVectorProperty
		);
}

void CBaseEmitterSpawnProperty::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_emitterSpawnPropertyGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_emitterSpawnPropertyCPU));
	m_emitterSpawnPropertyGPU->InitializeBuffer(device);
}

void CBaseEmitterSpawnProperty::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	if (m_isEmitterSpawnPropertyChanged)
	{
		m_emitterSpawnPropertyGPU->Stage(deviceContext);
		m_emitterSpawnPropertyGPU->Upload(deviceContext);
		m_isEmitterSpawnPropertyChanged = false;
	}
}

ID3D11Buffer* CBaseEmitterSpawnProperty::GetEmitterSpawnPropertyBuffer() const noexcept { return m_emitterSpawnPropertyGPU->GetBuffer(); }

void CBaseEmitterSpawnProperty::DrawPropertyUI()
{
	if (!ImGui::CollapsingHeader("이미터 생성 프로퍼티"))
		return;

	SeparatorText("파티클 정보 설정");
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

	SeparatorText("파티클 초기 위치 설정");
	m_positionShapedVectorSelector->SelectEnums(m_positionShapedVector);
	if (m_positionShapedVectorSelector->SetShapedVectorProperty(m_positionShapedVector))
	{
		m_isEmitterSpawnPropertyChanged = true;
	}

	SeparatorText("파티클 초기 속도 설정");
	m_speedShapedVectorSelector->SelectEnums(m_speedShapedVector);
	if (m_speedShapedVectorSelector->SetShapedVectorProperty(m_speedShapedVector))
	{
		m_isEmitterSpawnPropertyChanged = true;
	}

	SeparatorText("파티클 초기 색상 설정");
	if (ColorEdit4("파티클 초기 색상", m_emitterSpawnPropertyCPU.color.m128_f32))
	{
		m_isEmitterSpawnPropertyChanged = true;
	}
}

