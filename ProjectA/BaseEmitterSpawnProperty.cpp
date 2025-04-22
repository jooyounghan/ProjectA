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
	m_positionShapedVector(EShapedVector::Sphere),
	m_speedShapedVector(EShapedVector::Sphere),
	m_isImmortal(false)
{
	AutoZeroMemory(m_emitterSpawnPropertyCPU);
	m_emitterSpawnPropertyCPU.initialParticleCount = 0;
	m_emitterSpawnPropertyCPU.initialParticleLife = 1.f;
	m_emitterSpawnPropertyCPU.color = XMVectorSet(1.f, 1.f, 1.f, 1.f);

	m_positionShapedVectorSelector = make_unique<ShapedVectorSelector>(
		"�ʱ� ��ġ ����", "�ʱ� ������",
		m_emitterSpawnPropertyCPU.shapedPositionVectorProperty
	);

	m_speedShapedVectorSelector = make_unique<ShapedVectorSelector>(
		"�ʱ� �ӵ� ����", "�ʱ� �ӵ�",
		m_emitterSpawnPropertyCPU.shapedSpeedVectorProperty
		);
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

	if (ColorPicker3("�ʱ� ��ƼŬ ����", m_emitterSpawnPropertyCPU.color.m128_f32))
	{
		m_isEmitterSpawnPropertyChanged = true;
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
}

