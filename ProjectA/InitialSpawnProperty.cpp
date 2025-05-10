#include "InitialSpawnProperty.h"
#include "MacroUtilities.h"

#include "DynamicBuffer.h"
#include "ShapedVectorSelector.h"
#include "InitialPropertyDefinition.h"

#include <limits>

using namespace std;
using namespace D3D11;
using namespace DirectX;
using namespace ImGui;

CInitialSpawnProperty::CInitialSpawnProperty()
	: m_isEmitterSpawnPropertyChanged(false),
	m_positionShapedVector(InitPositionShapedVector),
	m_positionOrigin(InitOrigin),
	m_positionUpVector(InitUpVector),
	m_speedShapedVector(InitSpeedShapedVector),
	m_speedOrigin(InitOrigin),
	m_speedUpVector(InitUpVector),
	m_isImmortal(false)
{
	ZeroMem(m_emitterSpawnPropertyCPU);
	m_emitterSpawnPropertyCPU.initialParticleCount = 0;
	m_emitterSpawnPropertyCPU.initialParticleLife = InitLife;
	m_emitterSpawnPropertyCPU.xyScale = InitXYScale;

	m_positionShapedVectorSelector = make_unique<CShapedVectorSelector>(
		"�ʱ� ��ġ ����", "�ʱ� ������",
		m_positionOrigin, m_positionUpVector, m_positionCenterAngle,
		m_emitterSpawnPropertyCPU.shapedPositionVectorProperty
	);

	m_speedShapedVectorSelector = make_unique<CShapedVectorSelector>(
		"�ʱ� �ӵ� ����", "�ʱ� �ӵ�",
		m_speedOrigin, m_speedUpVector, m_speedCenterAngle,
		m_emitterSpawnPropertyCPU.shapedSpeedVectorProperty
	);
}

void CInitialSpawnProperty::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_emitterSpawnPropertyGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_emitterSpawnPropertyCPU));
	m_emitterSpawnPropertyGPU->InitializeBuffer(device);
}

void CInitialSpawnProperty::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	if (m_isEmitterSpawnPropertyChanged)
	{
		m_emitterSpawnPropertyGPU->Stage(deviceContext);
		m_emitterSpawnPropertyGPU->Upload(deviceContext);
		m_isEmitterSpawnPropertyChanged = false;
	}
}

ID3D11Buffer* CInitialSpawnProperty::GetInitialSpawnPropertyBuffer() const noexcept { return m_emitterSpawnPropertyGPU->GetBuffer(); }

void CInitialSpawnProperty::DrawUI()
{
	if (!ImGui::CollapsingHeader("�̹��� ���� ������Ƽ"))
		return;

	DrawUIImpl();
}

void CInitialSpawnProperty::DrawUIImpl()
{
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
		m_emitterSpawnPropertyCPU.initialParticleLife = m_isImmortal ? numeric_limits<float>::max() : InitLife;
	}

	SeparatorText("�ʱ� ��ƼŬ ��ġ ����");
	m_isEmitterSpawnPropertyChanged |=
		m_positionShapedVectorSelector->SelectEnums(m_positionShapedVector) |
		m_positionShapedVectorSelector->SetShapedVectorProperty(m_positionShapedVector);

	SeparatorText("�ʱ� ��ƼŬ �ӵ� ����");
	m_isEmitterSpawnPropertyChanged |=
		m_speedShapedVectorSelector->SelectEnums(m_speedShapedVector) |
		m_speedShapedVectorSelector->SetShapedVectorProperty(m_speedShapedVector);


	BeginDisabled(!m_useInitialColor);
	SeparatorText("�ʱ� ��ƼŬ ���� ����");
	if (ColorEdit4("�ʱ� ��ƼŬ ����", m_emitterSpawnPropertyCPU.color.m128_f32))
	{
		m_isEmitterSpawnPropertyChanged = true;
	}
	EndDisabled();

	BeginDisabled(!m_useInitialSize);
	SeparatorText("�ʱ� ��ƼŬ ũ��");
	if (DragFloat2("�ʱ� ��ƼŬ X-Y ũ��", &m_emitterSpawnPropertyCPU.xyScale.x, 0.01f, 0.f, 10.f, "%.2f"))
	{
		m_isEmitterSpawnPropertyChanged = true;
	}
	EndDisabled();
}

void CInitialSpawnProperty::Serialize(std::ofstream& ofs)
{
	SerializeHelper::SerializeElement<decltype(m_emitterSpawnPropertyCPU)>(ofs, m_emitterSpawnPropertyCPU);

	SerializeHelper::SerializeElement<EShapedVector>(ofs, m_positionShapedVector);
	SerializeHelper::SerializeElement<XMFLOAT3>(ofs, m_positionOrigin);
	SerializeHelper::SerializeElement<XMVECTOR>(ofs, m_positionUpVector);
	SerializeHelper::SerializeElement<float>(ofs, m_positionCenterAngle);

	SerializeHelper::SerializeElement<EShapedVector>(ofs, m_speedShapedVector);
	SerializeHelper::SerializeElement<XMFLOAT3>(ofs, m_speedOrigin);
	SerializeHelper::SerializeElement<XMVECTOR>(ofs, m_speedUpVector);
	SerializeHelper::SerializeElement<float>(ofs, m_speedCenterAngle);

	SerializeHelper::SerializeElement <bool>(ofs, m_isImmortal);
}

void CInitialSpawnProperty::Deserialize(std::ifstream& ifs)
{
	m_emitterSpawnPropertyCPU = SerializeHelper::DeserializeElement<decltype(m_emitterSpawnPropertyCPU)>(ifs);

	m_positionShapedVector = SerializeHelper::DeserializeElement<EShapedVector>(ifs);
	m_positionOrigin = SerializeHelper::DeserializeElement<XMFLOAT3>(ifs);
	m_positionUpVector = SerializeHelper::DeserializeElement<XMVECTOR>(ifs);
	m_positionCenterAngle = SerializeHelper::DeserializeElement<float>(ifs);

	m_speedShapedVector = SerializeHelper::DeserializeElement<EShapedVector>(ifs);
	m_speedOrigin = SerializeHelper::DeserializeElement<XMFLOAT3>(ifs);
	m_speedUpVector = SerializeHelper::DeserializeElement<XMVECTOR>(ifs);
	m_speedCenterAngle = SerializeHelper::DeserializeElement<float>(ifs);

	m_isImmortal = SerializeHelper::DeserializeElement <bool>(ifs);
	m_isEmitterSpawnPropertyChanged = true;
}

