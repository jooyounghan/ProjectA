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
		"�ʱ� ��ġ ����", "�ʱ� ������",
		m_emitterSpawnPropertyCPU.shapedPositionVectorProperty
	);

	m_speedShapedVectorSelector = make_unique<CShapedVectorSelector>(
		"�ʱ� �ӵ� ����", "�ʱ� �ӵ�",
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
	if (!ImGui::CollapsingHeader("�̹��� ���� ������Ƽ"))
		return;

	SeparatorText("��ƼŬ ���� ����");
	if (DragInt("�ʱ� ��ƼŬ ����", (int*)&m_emitterSpawnPropertyCPU.initialParticleCount, 1.f, 0, 100000))
	{
		m_isEmitterSpawnPropertyChanged = true;
	}

	BeginDisabled(m_isImmortal);
	{
		if (DragFloat("�ʱ� ��ƼŬ ����", &m_emitterSpawnPropertyCPU.initialParticleLife, 0.1f, 0.f, 1000.f, m_isImmortal ? "����" : "%.1f"))
		{
			m_isEmitterSpawnPropertyChanged = true;
		}
	}
	EndDisabled();
	SameLine();
	if (Checkbox("Immortal ����", &m_isImmortal))
	{
		m_emitterSpawnPropertyCPU.initialParticleLife = m_isImmortal ? numeric_limits<float>::max() : 1.f;
	}

	SeparatorText("��ƼŬ �ʱ� ��ġ ����");
	m_positionShapedVectorSelector->SelectEnums(m_positionShapedVector);
	if (m_positionShapedVectorSelector->SetShapedVectorProperty(m_positionShapedVector))
	{
		m_isEmitterSpawnPropertyChanged = true;
	}

	SeparatorText("��ƼŬ �ʱ� �ӵ� ����");
	m_speedShapedVectorSelector->SelectEnums(m_speedShapedVector);
	if (m_speedShapedVectorSelector->SetShapedVectorProperty(m_speedShapedVector))
	{
		m_isEmitterSpawnPropertyChanged = true;
	}

	SeparatorText("��ƼŬ �ʱ� ���� ����");
	if (ColorEdit4("��ƼŬ �ʱ� ����", m_emitterSpawnPropertyCPU.color.m128_f32))
	{
		m_isEmitterSpawnPropertyChanged = true;
	}
}

