#include "BaseParticleSpawnProperty.h"
#include "ControlPointGridView.h"
#include "MacroUtilities.h"
#include "BufferMacroUtilities.h"
#include "imgui.h"


using namespace std;
using namespace D3D11;
using namespace DirectX;
using namespace ImGui;

#define CreateLifeInterpolater()									\
m_lifeInterpolater = InterpolationSelector::CreateInterpolater<2>(	\
	m_lifeInterpolationMethod,										\
	m_lifeInitControlPoint, m_lifeFinalControlPoint,				\
	m_lifeControlPoints												\
)																	\

#define CreateColorInterpolater()									\
m_colorInterpolater = InterpolationSelector::CreateInterpolater<3>(	\
	m_colorInterpolationMethod,										\
	m_colorInitControlPoint, m_colorFinalControlPoint,				\
	m_colorControlPoints											\
)					

BaseParticleSpawnProperty::BaseParticleSpawnProperty(float& emitterCurrentTime, float& loopTime)
	: APropertyHasLoopTime(loopTime),
	m_emitterCurrentTime(emitterCurrentTime),
	m_lastLoopTime(loopTime),
	m_lifeInitControlPoint{ 0.f, MakeArray(8.f, 12.f)},
	m_lifeFinalControlPoint{ loopTime, MakeArray(0.f, 2.f)},
	m_lifeInterpolationMethod(EInterpolationMethod::Linear),
	m_colorInitControlPoint{ 0.f, MakeArray(0.f, 0.f, 0.f)},
	m_colorFinalControlPoint{ loopTime, MakeArray(1.f, 1.f ,1.f)},
	m_colorInterpolationMethod(EInterpolationMethod::Linear)
{
	AutoZeroMemory(m_baseParticleSpawnPropertyCPU);
	CreateLifeInterpolater();
	CreateColorInterpolater();

	m_origin = XMFLOAT3(0.f, 0.f, 0.f);
	m_upVector = XMVectorSet(0.f, 1.f, 0.f, 0.f);

	m_lifeControlPointGridView = make_unique<ControlPointGridView<2>>(
		"시간",
		array<string, 2>{ "최소 생명", "최대 생명" },
		"생명 주기",
		0.1f, 0.f, 20.f,
		m_lifeInitControlPoint,
		m_lifeFinalControlPoint,
		m_lifeControlPoints, true
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
		m_baseParticleSpawnPropertyCPU.color = XMFLOAT3(colorInterpolated[0], colorInterpolated[1], colorInterpolated[2]);
	}

	m_baseParticleSpawnPropertyGPU->Stage(deviceContext);
	m_baseParticleSpawnPropertyGPU->Upload(deviceContext);
}

void BaseParticleSpawnProperty::DrawPropertyUI()
{
	static EShapedVector shapedVector = EShapedVector::None;
	static bool isLifeInterpolaterChanged = false;
	static bool isColorInterpolaterChanged = false;

	if (m_lastLoopTime != m_loopTime)
	{
		AdjustControlPointsFromLoopTime();
		isLifeInterpolaterChanged = true;
		isColorInterpolaterChanged = true;
	}

	if (!ImGui::CollapsingHeader("파티클 생성 프로퍼티"))
		return;

	ShapedVectorSelector::SelectEnums("생성 속도 벡터", ShapedVectorSelector::GShapedVectorStringMaps, shapedVector);
	ShapedVectorSelector::SetShapedVectorProperty(m_origin, m_upVector, shapedVector, m_baseParticleSpawnPropertyCPU.shapedSpeedVectorSelector);

	EInterpolationMethod currnetLifeInterpolateKind = m_lifeInterpolationMethod;
	InterpolationSelector::SelectEnums("생성 파티클 생명 보간 방법", InterpolationSelector::GInterpolationMethodStringMap, currnetLifeInterpolateKind);
	if (m_lifeInterpolationMethod != currnetLifeInterpolateKind)
	{
		m_lifeInterpolationMethod = currnetLifeInterpolateKind;
		isLifeInterpolaterChanged = true;
	}

	if (m_lifeControlPointGridView->DrawControlPointGridView())
	{
		isLifeInterpolaterChanged = true;
	}

	InterpolationSelector::ViewInterpolatedPoints<2>(
		m_lifeInterpolater.get(),
		"Life Control Points",
		{ "최소 생명", "최대 생명" },
		m_lifeInitControlPoint,
		m_lifeFinalControlPoint,
		m_lifeControlPoints
	);

	if (isLifeInterpolaterChanged)
	{
		CreateLifeInterpolater();
		isLifeInterpolaterChanged = false;
	}

	EInterpolationMethod currnetColorInterpolateKind = m_colorInterpolationMethod;
	InterpolationSelector::SelectEnums("파티클 색상 보간 방법", InterpolationSelector::GInterpolationMethodStringMap, currnetColorInterpolateKind);
	if (m_colorInterpolationMethod != currnetColorInterpolateKind)
	{
		m_colorInterpolationMethod = currnetColorInterpolateKind;
		isColorInterpolaterChanged = true;
	}

	if (m_colorControlPointGridView->DrawControlPointGridView())
	{
		isColorInterpolaterChanged = true;
	}

	InterpolationSelector::ViewInterpolatedPoints<3>(
		m_colorInterpolater.get(),
		"Color Control Points",
		{ "R", "G", "B" },
		m_colorInitControlPoint,
		m_colorFinalControlPoint,
		m_colorControlPoints
	);

	if (isColorInterpolaterChanged)
	{
		CreateColorInterpolater();
		isColorInterpolaterChanged = false;
	}
}


