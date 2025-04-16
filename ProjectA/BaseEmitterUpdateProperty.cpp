#include "BaseEmitterUpdateProperty.h"
#include <exception>

#include "imgui.h"

using namespace std;
using namespace DirectX;
using namespace ImGui;

BaseEmitterUpdateProperty::BaseEmitterUpdateProperty(
	float& emitterCurrentTimeRef,
	UINT8 loopCount, 
	float loopTime,
	const std::vector<SControlPoint1>& spawnControlPoints /*= std::vector<SControlPoint>()*/,
	EInterpolationMethod spawnRateInterpolationMethod /*= EInterpolationMethod::Linear */
)
	: m_emitterCurrentTimeRef(emitterCurrentTimeRef),
	m_loopCount(loopCount),
	m_loopTime(loopTime),
	m_spawnControlPoints(spawnControlPoints),
	m_spawnRateInterpolationMethod(spawnRateInterpolationMethod),
	m_spawnRateInterpolater(InterpolateHelper::GetPoint1Interpolater(m_spawnRateInterpolationMethod)),
	m_isNotDisposed(true)
{

}


float BaseEmitterUpdateProperty::GetSpawnRate() const
{
	return m_spawnRateInterpolater(m_spawnControlPoints, m_emitterCurrentTimeRef);
}

void BaseEmitterUpdateProperty::SetSpawnControlPoints(const std::vector<SControlPoint1>& spawnControlPoints) noexcept
{
	m_spawnControlPoints = spawnControlPoints;
	
	// Interpolate
	m_spawnRateInterpolater(m_spawnControlPoints, 0.f);
}

void BaseEmitterUpdateProperty::SetSpawnRateInterpolationMethod(EInterpolationMethod spawnRateInterpolationMethod)
{
	m_spawnRateInterpolater = InterpolateHelper::GetPoint1Interpolater(spawnRateInterpolationMethod);
	m_spawnRateInterpolationMethod = spawnRateInterpolationMethod;
}

void BaseEmitterUpdateProperty::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{

}

void BaseEmitterUpdateProperty::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	if (m_loopCount > 0)
	{
		m_emitterCurrentTimeRef += dt;

		if (m_loopTime < m_emitterCurrentTimeRef)
		{
			if (m_loopCount == LoopInfinity)
			{

			}
			else
			{
				m_emitterCurrentTimeRef = max(m_emitterCurrentTimeRef - m_loopTime, 0.f);
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

}

std::unique_ptr<BaseEmitterUpdateProperty> BaseEmitterUpdateProperty::DrawPropertyCreator()
{
	SeparatorText("이미터 업데이트 프로퍼티");

	return nullptr;
}
