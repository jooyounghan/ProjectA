#include "ARuntimeSpawnProperty.h"
#include "MacroUtilities.h"
#include "BufferMacroUtilities.h"
#include "InitialPropertyDefinition.h"

#include "DynamicBuffer.h"

using namespace std;
using namespace D3D11;
using namespace DirectX;
using namespace ImGui;		

ARuntimeSpawnProperty::ARuntimeSpawnProperty(uint32_t maxEmitterCount)
	: IProperty(),
	m_currentLifeTime(0.f),
	m_isParticleSpawnPropertyChanged(false),
	m_positionShapedVector(InitPositionShapedVector),
	m_positionOrigin(InitOrigin),
	m_positionUpVector(InitUpVector),
	m_speedShapedVector(InitSpeedShapedVector),
	m_speedOrigin(InitOrigin),
	m_speedUpVector(InitUpVector),
	m_colorInitControlPoint{ 0.f, MakeArray(1.f, 0.f, 0.f, 1.f)},
	m_colorFinalControlPoint{ InitLife, MakeArray(0.f, 0.f ,1.f, 1.f)},
	m_colorInterpolationMethod(EInterpolationMethod::Linear),
	m_checkGPUColorInterpolater(false)
{
	AutoZeroMemory(m_baseParticleSpawnPropertyCPU);

	m_baseParticleSpawnPropertyCPU.maxLife = InitLife;

	m_positionShapedVectorSelector = make_unique<CShapedVectorSelector>(
		"생성 위치 벡터", "생성 반지름",
		m_positionOrigin, m_positionUpVector,
		m_baseParticleSpawnPropertyCPU.shapedPositionVectorProperty
	);

	m_speedShapedVectorSelector = make_unique<CShapedVectorSelector>(
		"생성 속도 벡터", "생성 속도",
		m_speedOrigin, m_speedUpVector,
		m_baseParticleSpawnPropertyCPU.shapedSpeedVectorProperty
	);

	m_d1Dim4PorpertyManager = make_unique<CGPUInterpPropertyManager<4, 2>>(maxEmitterCount);
	m_d3Dim4PorpertyManager = make_unique<CGPUInterpPropertyManager<4, 4>>(maxEmitterCount);

	m_colorControlPointGridView = make_unique<CControlPointGridView<4>>(
		"시간",
		array<string, 4>{ "R", "G", "B", "A"},
		"색상값",
		0.01f, 0.f, 1.f,
		m_colorInitControlPoint,
		m_colorFinalControlPoint,
		m_colorControlPoints
	);

	m_colorInterpolationSelectPlotter = make_unique<CInterpolaterSelectPlotter<4>>(
		"파티클 색상 보간 방법",
		"Color Control Points",
		array<string, 4>{ "R", "G", "B", "A" },
		m_colorInitControlPoint,
		m_colorFinalControlPoint,
		m_colorControlPoints
	);

	m_colorInterpolationSelectPlotter->CreateInterpolater(
		m_d1Dim4PorpertyManager.get(), 
		m_d3Dim4PorpertyManager.get(), 
		m_colorInterpolationMethod, 
		m_colorInterpolater
	);
	m_colorInterpolationSelectPlotter->ResetXYScale();
}

ID3D11Buffer* ARuntimeSpawnProperty::GetParticleSpawnPropertyBuffer() const noexcept { return m_baseParticleSpawnPropertyGPU->GetBuffer(); }


void ARuntimeSpawnProperty::AdjustControlPointsFromLife()
{
	const float& maxLife = m_baseParticleSpawnPropertyCPU.maxLife;
	m_colorFinalControlPoint.x = maxLife;
	m_colorControlPoints.erase(
		std::remove_if(m_colorControlPoints.begin(), m_colorControlPoints.end(),
			[&](const SControlPoint<4>& p)
			{
				return p.x > maxLife;
			}),
		m_colorControlPoints.end()
	);

	m_colorInterpolater->UpdateCoefficient();
	m_colorInterpolationSelectPlotter->ResetXYScale();
	m_isParticleSpawnPropertyChanged = true;
}


void ARuntimeSpawnProperty::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	OnInterpolateInformationChagned();
	m_baseParticleSpawnPropertyGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_baseParticleSpawnPropertyCPU));
	m_baseParticleSpawnPropertyGPU->InitializeBuffer(device);

	m_d1Dim4PorpertyManager->Initialize(device, deviceContext);
	m_d3Dim4PorpertyManager->Initialize(device, deviceContext);
}

void ARuntimeSpawnProperty::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	m_currentLifeTime += dt;
	const float& life = m_baseParticleSpawnPropertyCPU.maxLife;
	if (m_currentLifeTime > life)
	{
		m_currentLifeTime = max(m_currentLifeTime - life, 0.f);
	}

	m_d1Dim4PorpertyManager->Update(deviceContext, dt);
	m_d3Dim4PorpertyManager->Update(deviceContext, dt);

	if (!m_checkGPUColorInterpolater)
	{
		array<float, 4> interpolatedColor = m_colorInterpolater->GetInterpolated(m_currentLifeTime);
		m_baseParticleSpawnPropertyCPU.color = XMVectorSet(interpolatedColor[0], interpolatedColor[1], interpolatedColor[2], interpolatedColor[3]);
		m_isParticleSpawnPropertyChanged = true;
	}

	if (m_isParticleSpawnPropertyChanged)
	{
		m_baseParticleSpawnPropertyGPU->Stage(deviceContext);
		m_baseParticleSpawnPropertyGPU->Upload(deviceContext);
		m_isParticleSpawnPropertyChanged = false;
	}
}

void ARuntimeSpawnProperty::DrawPropertyUI()
{
	if (!ImGui::CollapsingHeader("파티클 생성 프로퍼티"))
		return;

	DrawPropertyUIImpl();
}

void ARuntimeSpawnProperty::DrawPropertyUIImpl()
{
	SeparatorText("파티클 생성 위치 설정");
	m_positionShapedVectorSelector->SelectEnums(m_positionShapedVector);
	if (m_positionShapedVectorSelector->SetShapedVectorProperty(m_positionShapedVector))
	{
		m_isParticleSpawnPropertyChanged = true;
	}

	SeparatorText("파티클 생성 속도 설정");
	m_speedShapedVectorSelector->SelectEnums(m_speedShapedVector);
	if (m_speedShapedVectorSelector->SetShapedVectorProperty(m_speedShapedVector))
	{
		m_isParticleSpawnPropertyChanged = true;
	}

	SeparatorText("파티클 생명 주기 설정");
	if (DragFloat("파티클 생명 주기", &m_baseParticleSpawnPropertyCPU.maxLife, 0.1f, 0.f, 10.f, "%.1f"))
	{
		AdjustControlPointsFromLife();
		OnInterpolateInformationChagned();
	}

	SeparatorText("파티클 색상 설정");
	EInterpolationMethod currnetColorInterpolateKind = m_colorInterpolationMethod;
	m_colorInterpolationSelectPlotter->SelectEnums(currnetColorInterpolateKind);
	if (m_colorInterpolationMethod != currnetColorInterpolateKind)
	{
		m_colorInterpolationMethod = currnetColorInterpolateKind;
		m_colorInterpolationSelectPlotter->CreateInterpolater(
			m_d1Dim4PorpertyManager.get(),
			m_d3Dim4PorpertyManager.get(),
			m_colorInterpolationMethod, 
			m_colorInterpolater
		);
		OnInterpolateInformationChagned();
	}

	if (Checkbox("GPU 기반 색상 보간", &m_checkGPUColorInterpolater))
	{
		m_colorInterpolationSelectPlotter->CreateInterpolater(
			m_d1Dim4PorpertyManager.get(),
			m_d3Dim4PorpertyManager.get(),
			m_colorInterpolationMethod,
			m_colorInterpolater
		);
		OnInterpolateInformationChagned();
	}

	if (m_colorControlPointGridView->DrawControlPointGridView())
	{
		m_colorInterpolater->UpdateCoefficient();
		m_colorInterpolationSelectPlotter->ResetXYScale();
	}

	m_colorInterpolationSelectPlotter->ViewInterpolatedPlots();
}

void ARuntimeSpawnProperty::Serialize(std::ofstream& ofs)
{
	SerializeHelper::SerializeElement<decltype(m_baseParticleSpawnPropertyCPU)>(ofs, m_baseParticleSpawnPropertyCPU);

	SerializeHelper::SerializeElement<EShapedVector>(ofs, m_positionShapedVector);
	SerializeHelper::SerializeElement<XMFLOAT3>(ofs, m_positionOrigin);
	SerializeHelper::SerializeElement<XMVECTOR>(ofs, m_positionUpVector);

	SerializeHelper::SerializeElement<EShapedVector>(ofs, m_speedShapedVector);
	SerializeHelper::SerializeElement<XMFLOAT3>(ofs, m_speedOrigin);
	SerializeHelper::SerializeElement<XMVECTOR>(ofs, m_speedUpVector);

	SerializeHelper::SerializeElement<SControlPoint<4>>(ofs, m_colorInitControlPoint);
	SerializeHelper::SerializeElement<SControlPoint<4>>(ofs, m_colorFinalControlPoint);
	SerializeHelper::SerializeVector<SControlPoint<4>>(ofs, m_colorControlPoints);
	SerializeHelper::SerializeElement<EInterpolationMethod>(ofs, m_colorInterpolationMethod);

	SerializeHelper::SerializeElement<bool>(ofs, m_checkGPUColorInterpolater);
}

void ARuntimeSpawnProperty::Deserialize(std::ifstream& ifs)
{
	m_baseParticleSpawnPropertyCPU = SerializeHelper::DeserializeElement<decltype(m_baseParticleSpawnPropertyCPU)>(ifs);

	m_positionShapedVector = SerializeHelper::DeserializeElement<EShapedVector>(ifs);
	m_positionOrigin = SerializeHelper::DeserializeElement<XMFLOAT3>(ifs);
	m_positionUpVector = SerializeHelper::DeserializeElement<XMVECTOR>(ifs);

	m_speedShapedVector = SerializeHelper::DeserializeElement<EShapedVector>(ifs);
	m_speedOrigin = SerializeHelper::DeserializeElement<XMFLOAT3>(ifs);
	m_speedUpVector = SerializeHelper::DeserializeElement<XMVECTOR>(ifs);

	m_colorInitControlPoint = SerializeHelper::DeserializeElement<SControlPoint<4>>(ifs);
	m_colorFinalControlPoint = SerializeHelper::DeserializeElement<SControlPoint<4>>(ifs);
	m_colorControlPoints = SerializeHelper::DeserializeVector<SControlPoint<4>>(ifs);
	m_colorInterpolationMethod = SerializeHelper::DeserializeElement<EInterpolationMethod>(ifs);

	m_checkGPUColorInterpolater = SerializeHelper::DeserializeElement<bool>(ifs);

	AdjustControlPointsFromLife();
	m_colorInterpolationSelectPlotter->CreateInterpolater(
		m_d1Dim4PorpertyManager.get(),
		m_d3Dim4PorpertyManager.get(),
		m_colorInterpolationMethod,
		m_colorInterpolater
	);
	OnInterpolateInformationChagned();
}