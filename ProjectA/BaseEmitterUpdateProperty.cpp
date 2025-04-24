#include "BaseEmitterUpdateProperty.h"

#include "ControlPointGridView.h"
#include "InterpolaterSelector.h"

using namespace std;
using namespace DirectX;
using namespace ImGui;

CBaseEmitterUpdateProperty::CBaseEmitterUpdateProperty()
	: IProperty(),
	m_currentTime(0.f),
	m_loopTime(10.f),
	m_spawnCount(0.f),
	m_saturatedSpawnCount(0),
	m_isLoopInfinity(true),
	m_loopCount(LoopInfinity),
	m_spawnInitControlPoint{ 0.f, 0.f },
	m_spawnFinalControlPoint{ 10.f, 0.f },
	m_spawnRateInterpolationMethod(EInterpolationMethod::Linear),
	m_isNotDisposed(true)
{
	m_spawnRateControlPointGridView = make_unique<CControlPointGridView<1>>(
		"�ð�",
		array<string, 1>{ "Spawn Rate" },
		"���� ��������",
		1.f, 0.f, 10000.f,
		m_spawnInitControlPoint,
		m_spawnFinalControlPoint,
		m_spawnControlPoints, false
	);

	m_spawnRateInterpolaterSelectPlotter = make_unique<CInterpolaterSelectPlotter<1>>(
		"���� �������� ���� ���",
		"Spawn Control Points",
		std::array<std::string, 1>{ "Spawn Rate" },
		m_spawnInitControlPoint,
		m_spawnFinalControlPoint,
		m_spawnControlPoints
	);

	m_spawnRateInterpolaterSelectPlotter->CreateInterpolater(false, m_spawnRateInterpolationMethod, m_spawnRateInterpolater);
	m_spawnRateInterpolaterSelectPlotter->RedrawSelectPlotter();
}


void CBaseEmitterUpdateProperty::AdjustControlPointsFromLoopTime()
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
	m_spawnRateInterpolater->UpdateCoefficient();
	m_spawnRateInterpolaterSelectPlotter->RedrawSelectPlotter();
}

void CBaseEmitterUpdateProperty::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{

}

void CBaseEmitterUpdateProperty::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	if (m_loopCount > 0)
	{
		m_currentTime += dt;
		m_spawnCount += max(0.f, m_spawnRateInterpolater->GetInterpolated(m_currentTime)[0]) * dt;
		m_saturatedSpawnCount = static_cast<UINT>(std::trunc(m_spawnCount));
		m_spawnCount = m_spawnCount - m_saturatedSpawnCount;

		if (m_currentTime > m_loopTime)
		{
			m_currentTime = max(m_currentTime - m_loopTime, 0.f);
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

void CBaseEmitterUpdateProperty::DrawPropertyUI()
{
	if (!ImGui::CollapsingHeader("�̹��� ������Ʈ ������Ƽ"))
		return;

	SeparatorText("�̹��� ���� ����");
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

	SeparatorText("��ƼŬ Spawn Rate ����");
	EInterpolationMethod currentSpawnRateInterpolateKind = m_spawnRateInterpolationMethod;
	m_spawnRateInterpolaterSelectPlotter->SelectEnums(currentSpawnRateInterpolateKind);
	if (m_spawnRateInterpolationMethod != currentSpawnRateInterpolateKind)
	{
		m_spawnRateInterpolationMethod = currentSpawnRateInterpolateKind;
		m_spawnRateInterpolaterSelectPlotter->CreateInterpolater(false, m_spawnRateInterpolationMethod, m_spawnRateInterpolater);
	}

	if (m_spawnRateControlPointGridView->DrawControlPointGridView())
	{
		m_spawnRateInterpolater->UpdateCoefficient();
		m_spawnRateInterpolaterSelectPlotter->RedrawSelectPlotter();
	}

	m_spawnRateInterpolaterSelectPlotter->ViewInterpolatedPlots();
}
