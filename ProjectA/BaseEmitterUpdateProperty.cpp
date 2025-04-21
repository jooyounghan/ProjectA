#include "BaseEmitterUpdateProperty.h"
#include "ControlPointGridView.h"
#include <math.h>

using namespace std;
using namespace DirectX;
using namespace ImGui;

BaseEmitterUpdateProperty::BaseEmitterUpdateProperty(float& emitterCurrentTime, float& loopTime)
	: APropertyHasLoopTime(loopTime),
	m_emitterCurrentTime(emitterCurrentTime),
	m_spawnCount(0.f),
	m_saturatedSpawnCount(0),
	m_isLoopInfinity(true),
	m_loopCount(LoopInfinity),
	m_spawnInitControlPoint{ 0.f, 0.f },
	m_spawnFinalControlPoint{ 10.f, 0.f },
	m_spawnRateInterpolationMethod(EInterpolationMethod::Linear),
	m_isNotDisposed(true)
{
	m_spawnRateControlPointGridView = make_unique<ControlPointGridView<1>>(
		"�ð�",
		array<string, 1>{ "Spawn Rate" },
		"���� ��������",
		1.f, 0.f, 10000.f,
		m_spawnInitControlPoint,
		m_spawnFinalControlPoint,
		m_spawnControlPoints, false
	);

	m_spawnRateInterpolaterSelectPlotter = make_unique<InterpolationSelectPlotter<1>>(
		"���� �������� ���� ���",
		"Spawn Control Points",
		std::array<std::string, 1>{ "Spawn Rate" },
		m_spawnInitControlPoint,
		m_spawnFinalControlPoint,
		m_spawnControlPoints
	);

	m_spawnRateInterpolaterSelectPlotter->SetInterpolater(m_spawnRateInterpolationMethod, m_spawnRateInterpolater);
	m_spawnRateInterpolaterSelectPlotter->UpdateControlPoints(m_spawnRateInterpolater.get());
}


void BaseEmitterUpdateProperty::AdjustControlPointsFromLoopTime()
{
	m_spawnFinalControlPoint.x = m_loopTime;
	m_spawnControlPoints.erase(
		std::remove_if(m_spawnControlPoints.begin(), m_spawnControlPoints.end(),
			[&](const SControlPoint<1>& p)
			{
				return p.x > m_loopTime;
			}),
		m_spawnControlPoints.end()
	);
	m_spawnRateInterpolaterSelectPlotter->UpdateControlPoints(m_spawnRateInterpolater.get());
}

void BaseEmitterUpdateProperty::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{

}

void BaseEmitterUpdateProperty::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	if (m_loopCount > 0)
	{
		m_emitterCurrentTime += dt;
		m_spawnCount += max(0.f, m_spawnRateInterpolater->GetInterpolated(m_emitterCurrentTime)[0]) * dt;
		m_saturatedSpawnCount = static_cast<UINT>(std::trunc(m_spawnCount));
		m_spawnCount = m_spawnCount - m_saturatedSpawnCount;

		if (m_loopTime < m_emitterCurrentTime)
		{
			m_emitterCurrentTime = max(m_emitterCurrentTime - m_loopTime, 0.f);
			if (m_loopCount == LoopInfinity)
			{

			}
			else
			{
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
	if (!ImGui::CollapsingHeader("�̹��� ������Ʈ ������Ƽ"))
		return;

	BeginDisabled(m_isLoopInfinity);
	{
		DragInt("���� Ƚ��", (int*)&m_loopCount, 1.f, 0, LoopInfinity - 1, m_isLoopInfinity ? "����" : "%d");
		EndDisabled();
	}
	SameLine();
	if (Checkbox("���� ���� ����", &m_isLoopInfinity))
	{
		m_loopCount = m_isLoopInfinity ? LoopInfinity : 1;
	}

	if (DragFloat("���� �� �ð�", &m_loopTime, 0.1f, 0.f, 100.f, "%.1f"))
	{
		AdjustControlPointsFromLoopTime();
	}

	EInterpolationMethod currentSpawnRateInterpolateKind = m_spawnRateInterpolationMethod;
	m_spawnRateInterpolaterSelectPlotter->SelectEnums(currentSpawnRateInterpolateKind);
	if (m_spawnRateInterpolationMethod != currentSpawnRateInterpolateKind)
	{
		m_spawnRateInterpolationMethod = currentSpawnRateInterpolateKind;
		m_spawnRateInterpolaterSelectPlotter->SetInterpolater(m_spawnRateInterpolationMethod, m_spawnRateInterpolater);
	}

	if (m_spawnRateControlPointGridView->DrawControlPointGridView())
	{
		m_spawnRateInterpolaterSelectPlotter->UpdateControlPoints(m_spawnRateInterpolater.get());
	}

	m_spawnRateInterpolaterSelectPlotter->ViewInterpolatedPlots();
}
