#include "BaseParticleSpawnProperty.h"
#include "MacroUtilities.h"
#include "BufferMacroUtilities.h"
#include "ControlPointGridView.h"

using namespace std;
using namespace D3D11;
using namespace DirectX;
using namespace ImGui;		

BaseParticleSpawnProperty::BaseParticleSpawnProperty(float& emitterCurrentTime, float& loopTime)
	: APropertyHasLoopTime(loopTime),
	m_emitterCurrentTime(emitterCurrentTime),
	m_lastLoopTime(loopTime),
	m_positionShapedVector(EShapedVector::Manual),
	m_speedShapedVector(EShapedVector::Manual),
	m_lifeInitControlPoint{ 0.f, MakeArray(8.f, 12.f)},
	m_lifeFinalControlPoint{ loopTime, MakeArray(0.f, 2.f)},
	m_lifeInterpolationMethod(EInterpolationMethod::Linear),
	m_colorInitControlPoint{ 0.f, MakeArray(0.f, 0.f, 0.f)},
	m_colorFinalControlPoint{ loopTime, MakeArray(1.f, 1.f ,1.f)},
	m_colorInterpolationMethod(EInterpolationMethod::Linear)
{
	AutoZeroMemory(m_baseParticleSpawnPropertyCPU);
	m_baseParticleSpawnPropertyCPU.color = XMVectorSet(1.f, 1.f, 1.f, 1.f);

	m_positionShapedVectorSelector = make_unique<ShapedVectorSelector>(
		"생성 위치 벡터", "반지름",
		m_baseParticleSpawnPropertyCPU.shapedPositionVectorProperty
	);


	m_speedShapedVectorSelector = make_unique<ShapedVectorSelector>(
		"생성 속도 벡터", "생성 속도",
		m_baseParticleSpawnPropertyCPU.shapedSpeedVectorProperty
	);

	m_lifeControlPointGridView = make_unique<ControlPointGridView<2>>(
		"시간",
		array<string, 2>{ "최소 생명", "최대 생명" },
		"생명 주기",
		0.1f, 0.f, 20.f,
		m_lifeInitControlPoint,
		m_lifeFinalControlPoint,
		m_lifeControlPoints, true
	);

	m_lifeInterpolationSelectPlotter = make_unique<InterpolationSelectPlotter<2>>(
		"생성 파티클 생명 보간 방법",
		"Life Control Points",
		array<string, 2>{ "최소 생명", "최대 생명" },
		m_lifeInitControlPoint,
		m_lifeFinalControlPoint,
		m_lifeControlPoints
	);

	m_colorControlPointGridView = make_unique<ControlPointGridView<3>>(
		"시간",
		array<string, 3>{ "R", "G", "B" },
		"색상값",
		0.01f, 0.f, 1.f,
		m_colorInitControlPoint,
		m_colorFinalControlPoint,
		m_colorControlPoints
	);

	m_colorInterpolationSelectPlotter = make_unique<InterpolationSelectPlotter<3>>(
		"파티클 색상 보간 방법",
		"Color Control Points",
		array<string, 3>{ "R", "G", "B" },
		m_colorInitControlPoint,
		m_colorFinalControlPoint,
		m_colorControlPoints
	);

	m_lifeInterpolationSelectPlotter->SetInterpolater(m_lifeInterpolationMethod, m_lifeInterpolater);
	m_lifeInterpolationSelectPlotter->UpdateControlPoints(m_lifeInterpolater.get());

	m_colorInterpolationSelectPlotter->SetInterpolater(m_colorInterpolationMethod, m_colorInterpolater);
	m_colorInterpolationSelectPlotter->UpdateControlPoints(m_colorInterpolater.get());
}

void BaseParticleSpawnProperty::AdjustControlPointsFromLoopTime()
{
	m_lifeFinalControlPoint.x = m_loopTime;
	m_colorFinalControlPoint.x = m_loopTime;

	m_lifeControlPoints.erase(
		std::remove_if(m_lifeControlPoints.begin(), m_lifeControlPoints.end(),
			[&](const SControlPoint<2>& p)
			{
				return p.x > m_loopTime;
			}),
		m_lifeControlPoints.end()
	);

	m_colorControlPoints.erase(
		std::remove_if(m_colorControlPoints.begin(), m_colorControlPoints.end(),
			[&](const SControlPoint<3>& p)
			{
				return p.x > m_loopTime;
			}),
		m_colorControlPoints.end()
	);

	m_lastLoopTime = m_loopTime;

	m_lifeInterpolationSelectPlotter->UpdateControlPoints(m_lifeInterpolater.get());
	m_colorInterpolationSelectPlotter->UpdateControlPoints(m_colorInterpolater.get());
}

void BaseParticleSpawnProperty::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_baseParticleSpawnPropertyGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_baseParticleSpawnPropertyCPU));
	m_baseParticleSpawnPropertyGPU->InitializeBuffer(device);
}

void BaseParticleSpawnProperty::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	if (m_lifeInterpolater)
	{
		array<float, 2> minMaxLifeInterpolated = m_lifeInterpolater->GetInterpolated(m_emitterCurrentTime);
		m_baseParticleSpawnPropertyCPU.minMaxLifeTime = XMFLOAT2(minMaxLifeInterpolated[0], minMaxLifeInterpolated[1]);
	}

	if (m_colorInterpolater)
	{
		array<float, 3> colorInterpolated = m_colorInterpolater->GetInterpolated(m_emitterCurrentTime);
		m_baseParticleSpawnPropertyCPU.color = XMVectorSet(colorInterpolated[0], colorInterpolated[1], colorInterpolated[2], 1.f);
	}

	m_baseParticleSpawnPropertyGPU->Stage(deviceContext);
	m_baseParticleSpawnPropertyGPU->Upload(deviceContext);
}

void BaseParticleSpawnProperty::DrawPropertyUI()
{
	if (m_lastLoopTime != m_loopTime)
	{
		AdjustControlPointsFromLoopTime();
	}

	if (!ImGui::CollapsingHeader("파티클 생성 프로퍼티"))
		return;

	m_positionShapedVectorSelector->SelectEnums(m_positionShapedVector);
	m_positionShapedVectorSelector->SetShapedVectorProperty(m_positionShapedVector);

	m_speedShapedVectorSelector->SelectEnums(m_speedShapedVector);
	m_speedShapedVectorSelector->SetShapedVectorProperty(m_speedShapedVector);

	EInterpolationMethod currnetLifeInterpolateKind = m_lifeInterpolationMethod;
	m_lifeInterpolationSelectPlotter->SelectEnums(currnetLifeInterpolateKind);
	if (m_lifeInterpolationMethod != currnetLifeInterpolateKind)
	{
		m_lifeInterpolationMethod = currnetLifeInterpolateKind;
		m_lifeInterpolationSelectPlotter->SetInterpolater(m_lifeInterpolationMethod, m_lifeInterpolater);
	}

	if (m_lifeControlPointGridView->DrawControlPointGridView())
	{
		m_lifeInterpolationSelectPlotter->UpdateControlPoints(m_lifeInterpolater.get());
	}
	m_lifeInterpolationSelectPlotter->ViewInterpolatedPlots();

	EInterpolationMethod currnetColorInterpolateKind = m_colorInterpolationMethod;
	m_colorInterpolationSelectPlotter->SelectEnums(currnetColorInterpolateKind);
	if (m_colorInterpolationMethod != currnetColorInterpolateKind)
	{
		m_colorInterpolationMethod = currnetColorInterpolateKind;
		m_colorInterpolationSelectPlotter->SetInterpolater(m_colorInterpolationMethod, m_colorInterpolater);
	}

	if (m_colorControlPointGridView->DrawControlPointGridView())
	{
		m_colorInterpolationSelectPlotter->UpdateControlPoints(m_colorInterpolater.get());
	}
	m_colorInterpolationSelectPlotter->ViewInterpolatedPlots();
}


