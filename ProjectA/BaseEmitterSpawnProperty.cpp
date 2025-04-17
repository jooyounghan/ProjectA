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
}
//
//std::unique_ptr<BaseEmitterSpawnProperty> BaseEmitterSpawnProperty::DrawPropertyCreator(bool& isApplied)
//{
//	static UINT initialParticleCount = 0;
//	static bool isImmortal = true;
//	static float initialParticleLife = 1.f;
//	static SShapedVectorProperty shapedVectorSelector;
//	static EShapedVector positionShapedVector = EShapedVector::None;
//	static bool isChanged = true;
//
//	bool makeProperty = false;
//
//	if (!ImGui::CollapsingHeader("�̹��� ���� ������Ƽ"))
//		return nullptr;
//
//	BeginDisabled(isApplied);
//	{	
//		isChanged |= DragInt("�ʱ� ��ƼŬ ����", (int*)&initialParticleCount, 1.f, 0, 100000);
//
//		BeginDisabled(isImmortal);
//		isChanged |= DragFloat("�ʱ� ��ƼŬ ����", &initialParticleLife, 0.1f, 0.f, 1000.f);
//		EndDisabled();
//		SameLine();
//		Checkbox("Immortal ����", &isImmortal);
//
//		EShapedVector lastShapedVectorKind = positionShapedVector;
//		ShapedVectorSelector::SelectEnums("��ġ �ʱ� ��ġ", ShapedVectorSelector::GShapedVectorStringMaps, positionShapedVector);
//		if (lastShapedVectorKind != positionShapedVector) isChanged = true;
//
//		isChanged |= ShapedVectorSelector::SetShapedVectorProperty(positionShapedVector, shapedVectorSelector);
//		EndDisabled();
//	}
//
//	BeginDisabled(!isChanged);
//	{
//		if (Button("�̹��� ���� ������Ƽ ����"))
//		{
//			isChanged = false;
//			isApplied = true;
//			makeProperty = true;
//		}
//		else
//		{
//
//		}
//		EndDisabled();
//	}
//	SameLine();
//	BeginDisabled(isChanged);
//	{
//		if (Button("�̹��� ���� ������Ƽ �缳��"))
//		{
//			isChanged = true;
//			isApplied = false;
//		}
//		else
//		{
//
//		}
//		EndDisabled();
//	}
//
//	if (makeProperty)
//	{
//		return make_unique<BaseEmitterSpawnProperty>(
//			shapedVectorSelector, initialParticleCount, 
//			isImmortal ? numeric_limits<float>::max() : initialParticleLife
//		);
//	}
//	return nullptr;
//}
//
//
