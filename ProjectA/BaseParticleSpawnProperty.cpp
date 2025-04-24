#include "BaseParticleSpawnProperty.h"
#include "MacroUtilities.h"
#include "BufferMacroUtilities.h"

#include "DynamicBuffer.h"

#include "ShapedVectorSelector.h"
#include "ControlPointGridView.h"
#include "InterpolaterSelector.h"


using namespace std;
using namespace D3D11;
using namespace DirectX;
using namespace ImGui;		

#define InitLife 1.f

CBaseParticleSpawnProperty::CBaseParticleSpawnProperty(
	float& emitterCurrentTime,
	float& emitterLoopTime,
	const function<void(float)>& lifeChangedHandler,
	const function<void(uint32_t, uint32_t)>& colorInterpolationChangedHandler
)
	: APropertyOnEmitterTimeline(emitterCurrentTime, emitterLoopTime),
	m_onLifeChanged(lifeChangedHandler),
	m_onColorInterpolationChanged(colorInterpolationChangedHandler),
	m_isParticleSpawnPropertyChanged(false),
	m_positionShapedVector(EShapedVector::Sphere),
	m_speedShapedVector(EShapedVector::None),
	m_colorInitControlPoint{ 0.f, MakeArray(1.f, 0.f, 0.f, 1.f)},
	m_colorFinalControlPoint{ InitLife, MakeArray(0.f, 0.f ,1.f, 1.f)},
	m_colorInterpolationMethod(EInterpolationMethod::Linear),
	m_useGPUColorInterpolater(true)
{
	AutoZeroMemory(m_baseParticleSpawnPropertyCPU);

	m_baseParticleSpawnPropertyCPU.life = InitLife;
	m_lastParticleLife = InitLife;

	m_positionShapedVectorSelector = make_unique<CShapedVectorSelector>(
		"생성 위치 벡터", "생성 반지름",
		m_baseParticleSpawnPropertyCPU.shapedPositionVectorProperty
	);

	m_speedShapedVectorSelector = make_unique<CShapedVectorSelector>(
		"생성 속도 벡터", "생성 속도",
		m_baseParticleSpawnPropertyCPU.shapedSpeedVectorProperty
	);

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

	m_colorInterpolationSelectPlotter->CreateInterpolater(m_useGPUColorInterpolater, m_colorInterpolationMethod, m_colorInterpolater);
	m_colorInterpolationSelectPlotter->RedrawSelectPlotter();

	m_onLifeChanged(InitLife);
	m_onColorInterpolationChanged(m_colorInterpolater->GetInterpolaterID(), m_colorInterpolater->GetCoefficientCount());
}

ID3D11Buffer* CBaseParticleSpawnProperty::GetParticleSpawnPropertyBuffer() const noexcept { return m_baseParticleSpawnPropertyGPU->GetBuffer(); }

void CBaseParticleSpawnProperty::AdjustControlPointsFromLoopTime()
{
	const float& loopTime = m_emitterLoopTime;
	m_colorFinalControlPoint.x = loopTime;
	m_colorControlPoints.erase(
		std::remove_if(m_colorControlPoints.begin(), m_colorControlPoints.end(),
			[&](const SControlPoint<4>& p)
			{
				return p.x > loopTime;
			}),
		m_colorControlPoints.end()
				);

	m_lastParticleLife = loopTime;
	m_colorInterpolater->UpdateCoefficient();
	m_colorInterpolationSelectPlotter->RedrawSelectPlotter();
}

void CBaseParticleSpawnProperty::AdjustControlPointsFromLife()
{
	const float& life = m_baseParticleSpawnPropertyCPU.life;
	m_colorFinalControlPoint.x = life;
	m_colorControlPoints.erase(
		std::remove_if(m_colorControlPoints.begin(), m_colorControlPoints.end(),
			[&](const SControlPoint<4>& p)
			{
				return p.x > life;
			}),
		m_colorControlPoints.end()
	);

	m_lastParticleLife = life;
	m_colorInterpolater->UpdateCoefficient();
	m_colorInterpolationSelectPlotter->RedrawSelectPlotter();
}


void CBaseParticleSpawnProperty::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_baseParticleSpawnPropertyGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_baseParticleSpawnPropertyCPU));
	m_baseParticleSpawnPropertyGPU->InitializeBuffer(device);
}

void CBaseParticleSpawnProperty::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	if (!m_useGPUColorInterpolater)
	{
		array<float, 4> interpolatedColor = m_colorInterpolater->GetInterpolated(dt);
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

void CBaseParticleSpawnProperty::DrawPropertyUI()
{
	const float& life = m_baseParticleSpawnPropertyCPU.life;
	if (m_lastParticleLife - 1E-3 < life && life < m_lastParticleLife + 1E-3)
	{
	}
	else
	{
		AdjustControlPointsFromLife();
	}

	if (!ImGui::CollapsingHeader("파티클 생성 프로퍼티"))
		return;

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
	if (DragFloat("파티클 생명 주기", &m_baseParticleSpawnPropertyCPU.life, 0.1f, 0.f, 10.f, "%.1f"))
	{
		m_onLifeChanged(m_baseParticleSpawnPropertyCPU.life);
		m_isParticleSpawnPropertyChanged = true;
	}

	SeparatorText("파티클 색상 설정");
	EInterpolationMethod currnetColorInterpolateKind = m_colorInterpolationMethod;
	m_colorInterpolationSelectPlotter->SelectEnums(currnetColorInterpolateKind);
	if (m_colorInterpolationMethod != currnetColorInterpolateKind)
	{
		m_colorInterpolationMethod = currnetColorInterpolateKind;
		m_colorInterpolationSelectPlotter->CreateInterpolater(m_useGPUColorInterpolater, m_colorInterpolationMethod, m_colorInterpolater);
		m_onColorInterpolationChanged(m_colorInterpolater->GetInterpolaterID(), m_colorInterpolater->GetCoefficientCount());
	}
	if (Checkbox("GPU 기반 색상 보간", &m_useGPUColorInterpolater))
	{
		if (m_useGPUColorInterpolater)
		{
			m_baseParticleSpawnPropertyCPU.color = XMVectorZero();
			m_isParticleSpawnPropertyChanged = true;
		}
		else
		{

		}

		m_colorInterpolationSelectPlotter->CreateInterpolater(m_useGPUColorInterpolater, m_colorInterpolationMethod, m_colorInterpolater);
		m_onColorInterpolationChanged(m_colorInterpolater->GetInterpolaterID(), m_colorInterpolater->GetCoefficientCount());
	}

	if (m_colorControlPointGridView->DrawControlPointGridView())
	{
		m_colorInterpolater->UpdateCoefficient();
		m_colorInterpolationSelectPlotter->RedrawSelectPlotter();
	}
	m_colorInterpolationSelectPlotter->ViewInterpolatedPlots();
}


