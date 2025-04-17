#include "BaseEmitterSpawnProperty.h"
#include "BufferMacroUtilities.h"

#include <limits>

using namespace std;
using namespace D3D11;
using namespace DirectX;
using namespace ImGui;

BaseEmitterSpawnProperty::BaseEmitterSpawnProperty(
	const SShapedVectorProperty& shapedVectorProperty,
	UINT initialParticleCount,
	float initialParticleLife
)
	: m_emitterSpawnPropertyCPU{ shapedVectorProperty, initialParticleCount, initialParticleLife }
{
}

void BaseEmitterSpawnProperty::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_emitterSpawnPropertyGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_emitterSpawnPropertyCPU));
	m_emitterSpawnPropertyGPU->InitializeBuffer(device);
}

void BaseEmitterSpawnProperty::Update(ID3D11DeviceContext* deviceContext, float dt)
{

}

void BaseEmitterSpawnProperty::DrawPropertyUI()
{
}

std::unique_ptr<BaseEmitterSpawnProperty> BaseEmitterSpawnProperty::DrawPropertyCreator(bool& isApplied)
{
	static UINT initialParticleCount = 0;
	static bool isImmortal = true;
	static float initialParticleLife = 1.f;
	static SShapedVectorProperty shapedVectorSelector;
	static EShapedVector positionShapedVector = EShapedVector::None;
	static bool isChanged = true;

	bool makeProperty = false;

	if (!ImGui::CollapsingHeader("�̹��� ���� ������Ƽ"))
		return nullptr;

	BeginDisabled(isApplied);
	{	
		isChanged |= DragInt("�ʱ� ��ƼŬ ����", (int*)&initialParticleCount, 1.f, 0, 100000);

		BeginDisabled(isImmortal);
		isChanged |= DragFloat("�ʱ� ��ƼŬ ����", &initialParticleLife, 0.1f, 0.f, 1000.f);
		EndDisabled();
		SameLine();
		Checkbox("Immortal ����", &isImmortal);

		EShapedVector lastShapedVectorKind = positionShapedVector;
		ShapedVectorSelector::SelectEnums("��ġ �ʱ� ��ġ", ShapedVectorSelector::GShapedVectorStringMap, positionShapedVector);
		if (lastShapedVectorKind != positionShapedVector) isChanged = true;

		isChanged |= ShapedVectorSelector::SetShapedVectorProperty(positionShapedVector, shapedVectorSelector);
		EndDisabled();
	}

	BeginDisabled(!isChanged);
	{
		if (Button("�̹��� ���� ������Ƽ ����"))
		{
			isChanged = false;
			isApplied = true;
			makeProperty = true;
		}
		else
		{

		}
		EndDisabled();
	}
	SameLine();
	BeginDisabled(isChanged);
	{
		if (Button("�̹��� ���� ������Ƽ �缳��"))
		{
			isChanged = true;
			isApplied = false;
		}
		else
		{

		}
		EndDisabled();
	}

	if (makeProperty)
	{
		return make_unique<BaseEmitterSpawnProperty>(
			shapedVectorSelector, initialParticleCount, 
			isImmortal ? numeric_limits<float>::max() : initialParticleLife
		);
	}
	return nullptr;
}


