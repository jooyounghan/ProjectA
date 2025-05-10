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
		"초기 위치 벡터", "초기 반지름",
		m_positionOrigin, m_positionUpVector, m_positionCenterAngle,
		m_emitterSpawnPropertyCPU.shapedPositionVectorProperty
	);

	m_speedShapedVectorSelector = make_unique<CShapedVectorSelector>(
		"초기 속도 벡터", "초기 속도",
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
	if (!ImGui::CollapsingHeader("이미터 생성 프로퍼티"))
		return;

	DrawUIImpl();
}

void CInitialSpawnProperty::DrawUIImpl()
{
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
		m_emitterSpawnPropertyCPU.initialParticleLife = m_isImmortal ? numeric_limits<float>::max() : InitLife;
	}

	SeparatorText("초기 파티클 위치 설정");
	m_isEmitterSpawnPropertyChanged |=
		m_positionShapedVectorSelector->SelectEnums(m_positionShapedVector) |
		m_positionShapedVectorSelector->SetShapedVectorProperty(m_positionShapedVector);

	SeparatorText("초기 파티클 속도 설정");
	m_isEmitterSpawnPropertyChanged |=
		m_speedShapedVectorSelector->SelectEnums(m_speedShapedVector) |
		m_speedShapedVectorSelector->SetShapedVectorProperty(m_speedShapedVector);


	BeginDisabled(!m_useInitialColor);
	SeparatorText("초기 파티클 색상 설정");
	if (ColorEdit4("초기 파티클 색상", m_emitterSpawnPropertyCPU.color.m128_f32))
	{
		m_isEmitterSpawnPropertyChanged = true;
	}
	EndDisabled();

	BeginDisabled(!m_useInitialSize);
	SeparatorText("초기 파티클 크기");
	if (DragFloat2("초기 파티클 X-Y 크기", &m_emitterSpawnPropertyCPU.xyScale.x, 0.01f, 0.f, 10.f, "%.2f"))
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

