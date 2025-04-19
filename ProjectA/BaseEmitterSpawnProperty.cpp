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
	m_origin = XMFLOAT3(0.f, 0.f, 0.f);
	m_upVector = XMVectorSet(0.f, 1.f, 0.f, 0.f);

	m_shapedPositionSelector = make_unique<ShapedVectorSelector>(
		"초기 위치 벡터",
		m_origin,
		m_upVector,
		m_emitterSpawnPropertyCPU.shapedPositionVectorProperty
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
		m_emitterSpawnPropertyCPU.initialParticleLife = isImmortal ? numeric_limits<float>::max() : m_emitterSpawnPropertyCPU.initialParticleLife;
		if (DragFloat("초기 파티클 생명", &m_emitterSpawnPropertyCPU.initialParticleLife, 0.1f, 0.f, 1000.f, isImmortal ? "무한" : "%.1f"))
		{
			m_isEmitterSpawnPropertyChanged = true;
		}
	}
	EndDisabled();
	SameLine();
	Checkbox("Immortal 설정", &isImmortal);

	m_shapedPositionSelector->SelectEnums(shapedVector);
	if (m_shapedPositionSelector->SetShapedVectorProperty(shapedVector))
	{
		m_isEmitterSpawnPropertyChanged = true;
	}
}

