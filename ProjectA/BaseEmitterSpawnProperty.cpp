#include "BaseEmitterSpawnProperty.h"
#include "BufferMacroUtilities.h"
#include "ShapedVectorSelector.h"

#include "imgui.h"

using namespace std;
using namespace D3D11;
using namespace DirectX;
using namespace ImGui;

BaseEmitterSpawnProperty::BaseEmitterSpawnProperty(
	const DirectX::XMFLOAT2& minInitRadian, 
	const DirectX::XMFLOAT2& maxInitRadian, 
	const DirectX::XMFLOAT2& minMaxRadius, 
	UINT initialParticleCount,
	float initialParticleLife
)
	: m_emitterSpawnPropertyCPU{ minInitRadian, maxInitRadian, minMaxRadius, initialParticleCount, initialParticleLife }
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

std::unique_ptr<BaseEmitterSpawnProperty> BaseEmitterSpawnProperty::DrawPropertyCreator()
{
	static UINT initialParticleCount = 0;
	static float initialParticleLife = 1.f;
	static XMFLOAT2 minInitRadian = XMFLOAT2(0.f, 0.f);
	static XMFLOAT2 maxInitRadian = XMFLOAT2(0.f, 0.f);
	static XMFLOAT2 minMaxRadius = XMFLOAT2(0.f, 0.f);
	static bool isChanged = true;
	static bool isImmortal = false;
	EShapedVector positionShapedVector = EShapedVector::None;

	SeparatorText("이미터 생성 프로퍼티");
	isChanged |= DragInt("초기 파티클 개수", (int*)&initialParticleCount, 1.f, 0, 100000);

	BeginDisabled(isImmortal);
	isChanged |= DragFloat("초기 파티클 생명", &initialParticleLife, 0.1f, 0.f, 1000.f);
	EndDisabled();
	SameLine();
	Checkbox("초기 파티클 수명 여부", &isImmortal);

	ShapedVectorSelector::SelectShapedVector(positionShapedVector);

	BeginDisabled(!isChanged);
	Button("이미터 생성 프로퍼티 설정");
	EndDisabled();

	return nullptr;

}
