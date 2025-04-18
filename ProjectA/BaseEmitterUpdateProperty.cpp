#include "BaseEmitterUpdateProperty.h"
#include "ControlPointGridView.h"

using namespace std;
using namespace DirectX;
using namespace ImGui;

#define CreateSpawnRateInterpolater()									\
m_spawnRateInterpolater = InterpolationSelector::CreateInterpolater<1>(	\
	m_spawnRateInterpolationMethod,										\
	m_spawnInitControlPoint, m_spawnFinalControlPoint,					\
	m_spawnControlPoints												\
)																		\

BaseEmitterUpdateProperty::BaseEmitterUpdateProperty(float& emitterCurrentTime)
	: m_emitterCurrentTime(emitterCurrentTime),
	m_loopCount(LoopInfinity),
	m_loopTime(10.f),
	m_spawnInitControlPoint{ 0.f, 0.f },
	m_spawnFinalControlPoint{ 10.f, 0.f },
	m_spawnRateInterpolationMethod(EInterpolationMethod::Linear),
	m_isNotDisposed(true)
{
	CreateSpawnRateInterpolater();
}


float BaseEmitterUpdateProperty::GetSpawnRate() const
{
	return m_spawnRateInterpolater->GetInterpolated(m_emitterCurrentTime)[0];
}

void BaseEmitterUpdateProperty::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{

}

void BaseEmitterUpdateProperty::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	if (m_loopCount > 0)
	{
		m_emitterCurrentTime += dt;

		if (m_loopTime < m_emitterCurrentTime)
		{
			if (m_loopCount == LoopInfinity)
			{

			}
			else
			{
				m_emitterCurrentTime = max(m_emitterCurrentTime - m_loopTime, 0.f);
				m_loopCount -= 1;
			}
		}
	}
	else
	{
		if (m_isNotDisposed)
		{
			m_emitterDisposeHandler(this);
			m_isNotDisposed = false;
		}
	}
}

void BaseEmitterUpdateProperty::DrawPropertyUI()
{
	static bool isLoopInfinity = true;
	static bool isSpawnRateInterpolaterChanged = false;

	if (!ImGui::CollapsingHeader("이미터 업데이트 프로퍼티"))
		return;

	BeginDisabled(isLoopInfinity);
	{
		DragInt("루프 횟수", (int*)&m_loopCount, 1.f, 0, LoopInfinity - 1, isLoopInfinity ? "무한" : "%d");
		EndDisabled();
	}
	SameLine();
	if (Checkbox("무한 루프 설정", &isLoopInfinity))
	{
		m_loopCount = isLoopInfinity ? LoopInfinity : 1;
	}

	if (DragFloat("루프 당 시간", &m_loopTime, 0.1f, 0.f, 100.f, "%.1f"))
	{
		m_spawnFinalControlPoint.x = m_loopTime;
		isSpawnRateInterpolaterChanged = true;
	}

	EInterpolationMethod currentSpawnRateInterpolateKind = m_spawnRateInterpolationMethod;
	InterpolationSelector::SelectEnums("생성 프로파일 보간 방법", InterpolationSelector::GInterpolationMethodStringMap, currentSpawnRateInterpolateKind);
	if (m_spawnRateInterpolationMethod != currentSpawnRateInterpolateKind)
	{
		m_spawnRateInterpolationMethod = currentSpawnRateInterpolateKind;
		isSpawnRateInterpolaterChanged = true;
	}

	if (ControlPointGridView::HandleControlPointsGridView<1>(
		"시간", 
		{ "Spawn Rate" },
		"생성 프로파일",
		1.f, 0.f, 10000.f,
		m_spawnInitControlPoint,
		m_spawnFinalControlPoint,
		m_spawnControlPoints
	))
	{
		isSpawnRateInterpolaterChanged = true;
	}
	InterpolationSelector::ViewInterpolatedPoints<1>(
		m_spawnRateInterpolater.get(),
		"Spawn Control Points",
		{ "Spawn Rate" },
		m_spawnInitControlPoint,
		m_spawnFinalControlPoint,
		m_spawnControlPoints
	);

	if (isSpawnRateInterpolaterChanged)
	{
		CreateSpawnRateInterpolater();
		isSpawnRateInterpolaterChanged = false;
	}

}
