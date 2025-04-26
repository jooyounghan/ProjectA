#include "EmitterUpdateProperty.h"

#include "InitialPropertyDefinition.h"
#include "ControlPointGridView.h"
#include "InterpolaterSelector.h"

using namespace std;
using namespace DirectX;
using namespace ImGui;

CEmitterUpdateProperty::CEmitterUpdateProperty()
	: IProperty(),
	m_currentTime(0.f),
	m_loopTime(InitLife),
	m_spawnCount(0.f),
	m_saturatedSpawnCount(0),
	m_isLoopInfinity(true),
	m_spawnInitControlPoint{ 0.f, 0.f },
	m_spawnFinalControlPoint{ 10.f, 0.f },
	m_spawnRateInterpolationMethod(EInterpolationMethod::Linear),
	m_isNotDisposed(true)
{
	SetLoopCount(LoopInfinity);

	m_spawnRateControlPointGridView = make_unique<CControlPointGridView<1>>(
		"시간",
		array<string, 1>{ "Spawn Rate" },
		"생성 프로파일",
		1.f, 0.f, 10000.f,
		m_spawnInitControlPoint,
		m_spawnFinalControlPoint,
		m_spawnControlPoints, false
	);

	m_spawnRateInterpolaterSelectPlotter = make_unique<CInterpolaterSelectPlotter<1>>(
		"생성 프로파일 보간 방법",
		"Spawn Control Points",
		std::array<std::string, 1>{ "Spawn Rate" },
		m_spawnInitControlPoint,
		m_spawnFinalControlPoint,
		m_spawnControlPoints
	);

	m_spawnRateInterpolaterSelectPlotter->CreateInterpolater(false, m_spawnRateInterpolationMethod, m_spawnRateInterpolater);
	m_spawnRateInterpolaterSelectPlotter->ResetXYScale();
}

void CEmitterUpdateProperty::SetLoopCount(UINT8 loopCount)
{
	m_loopCount = loopCount;
	m_currentLoopCount = loopCount;
}


void CEmitterUpdateProperty::AdjustControlPointsFromLoopTime()
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
	m_spawnRateInterpolaterSelectPlotter->ResetXYScale();
}

void CEmitterUpdateProperty::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{

}

void CEmitterUpdateProperty::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	if (m_currentLoopCount > 0)
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
				m_currentLoopCount -= 1;
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

void CEmitterUpdateProperty::DrawPropertyUI()
{
	if (!ImGui::CollapsingHeader("이미터 업데이트 프로퍼티"))
		return;

	SeparatorText("이미터 루프 설정");
	BeginDisabled(m_isLoopInfinity);
	{
		if (DragInt("루프 횟수", (int*)&m_loopCount, 1.f, 0, LoopInfinity - 1, m_isLoopInfinity ? "무한" : "%d"))
		{
			SetLoopCount(m_loopCount);
		}
		EndDisabled();
	}
	SameLine();
	if (Checkbox("무한 루프 설정", &m_isLoopInfinity))
	{
		SetLoopCount(m_isLoopInfinity ? LoopInfinity : m_loopCount);
	}

	if (DragFloat("루프 당 시간", &m_loopTime, 0.1f, 0.f, 100.f, "%.1f"))
	{
		AdjustControlPointsFromLoopTime();
	}

	SeparatorText("파티클 Spawn Rate 설정");
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
		m_spawnRateInterpolaterSelectPlotter->ResetXYScale();
	}

	m_spawnRateInterpolaterSelectPlotter->ViewInterpolatedPlots();
}

void CEmitterUpdateProperty::Serialize(std::ofstream& ofs)
{
	SerializeHelper::SerializeElement<float>(ofs, m_loopTime);
	SerializeHelper::SerializeElement<bool>(ofs, m_isLoopInfinity);
	SerializeHelper::SerializeElement<UINT8>(ofs, m_loopCount);

	SerializeHelper::SerializeElement<SControlPoint<1>>(ofs, m_spawnInitControlPoint);
	SerializeHelper::SerializeElement<SControlPoint<1>>(ofs, m_spawnFinalControlPoint);
	SerializeHelper::SerializeVector<SControlPoint<1>>(ofs, m_spawnControlPoints);
	SerializeHelper::SerializeElement<EInterpolationMethod>(ofs, m_spawnRateInterpolationMethod);
}

void CEmitterUpdateProperty::Deserialize(std::ifstream& ifs)
{
	m_loopTime = SerializeHelper::DeserializeElement<float>(ifs);
	m_isLoopInfinity = SerializeHelper::DeserializeElement<bool>(ifs);
	UINT8 loopCount = SerializeHelper::DeserializeElement<UINT8>(ifs);
	SetLoopCount(loopCount);

	m_spawnInitControlPoint = SerializeHelper::DeserializeElement<SControlPoint<1>>(ifs);
	m_spawnFinalControlPoint= SerializeHelper::DeserializeElement<SControlPoint<1>>(ifs);
	m_spawnControlPoints = SerializeHelper::DeserializeVector<SControlPoint<1>>(ifs);
	m_spawnRateInterpolationMethod = SerializeHelper::DeserializeElement<EInterpolationMethod>(ifs);

	m_spawnRateInterpolaterSelectPlotter->CreateInterpolater(false, m_spawnRateInterpolationMethod, m_spawnRateInterpolater);
	m_spawnRateInterpolater->UpdateCoefficient();
	m_spawnRateInterpolaterSelectPlotter->ResetXYScale();
}
