#include "BaseParticleSpawnProperty.h"

#include "MacroUtilities.h"
#include "BufferMacroUtilities.h"

#include "imgui.h"

using namespace std;
using namespace D3D11;
using namespace DirectX;
using namespace ImGui;

BaseParticleSpawnProperty::BaseParticleSpawnProperty(float& emitterCurrentTime)
	: m_emitterCurrentTime(emitterCurrentTime)
{
	AutoZeroMemory(m_baseParticleSpawnPropertyCPU);
}

void BaseParticleSpawnProperty::SetMinMaxLifeTime(const DirectX::XMFLOAT2& minMaxLifeTime)
{
	m_baseParticleSpawnPropertyCPU.minMaxLifeTime = minMaxLifeTime;
	m_isParticleSpawnPropertyChanged = true;
}

void BaseParticleSpawnProperty::SetMinEmitRadian(const XMFLOAT2& minEmitRadian)
{
	m_baseParticleSpawnPropertyCPU.minEmitRadian = minEmitRadian;
	m_isParticleSpawnPropertyChanged = true;
}

void BaseParticleSpawnProperty::SetMaxEmitRadian(const XMFLOAT2& maxEmitRadian)
{
	m_baseParticleSpawnPropertyCPU.maxEmitRadian = maxEmitRadian;
	m_isParticleSpawnPropertyChanged = true;
}

void BaseParticleSpawnProperty::SetInitSpeedX(const SControlPoint& speedX) noexcept
{

}

void BaseParticleSpawnProperty::SetFinalSpeedX(const SControlPoint& speedX) noexcept
{

}

void BaseParticleSpawnProperty::SetSpeedXControlPoints(const std::vector<SControlPoint>& speedXControlPoints)
{
}

void BaseParticleSpawnProperty::SetInitSpeedY(const SControlPoint& speedY) noexcept
{

}

void BaseParticleSpawnProperty::SetFinalSpeedY(const SControlPoint& speedY) noexcept
{

}

void BaseParticleSpawnProperty::SetSpeedYControlPoints(const std::vector<SControlPoint>& speedXControlPoints)
{
}

void BaseParticleSpawnProperty::SetSpeedInterpolationMethod(EInterpolationMethod speedInterpolationMethod)
{
	//m_speedXInterpolater = InterpolaterHelper::GetInterpolater(speedInterpolationMethod);
	//m_speedYInterpolater = InterpolaterHelper::GetInterpolater(speedInterpolationMethod);
	m_speedInterpolationMethod = speedInterpolationMethod;
	m_isSpeedInterpolaterChanged = true;
}

void BaseParticleSpawnProperty::SetColorControlPoints(
	const vector<SControlPoint>& colorRControlPoints,
	const vector<SControlPoint>& colorGControlPoints, 
	const vector<SControlPoint>& colorBControlPoints
)
{
	m_colorRControlPoints = colorRControlPoints;
	m_colorGControlPoints = colorGControlPoints;
	m_colorBControlPoints = colorBControlPoints;
	m_isParticleSpawnPropertyChanged = true;
	m_isColorInterpolaterChanged = true;
}

void BaseParticleSpawnProperty::SetColorInterpolationMethod(EInterpolationMethod colorInterpolationMethod)
{
	//m_colorRInterpolater = InterpolaterHelper::GetInterpolater(colorInterpolationMethod);
	//m_colorGInterpolater = InterpolaterHelper::GetInterpolater(colorInterpolationMethod);
	//m_colorBInterpolater = InterpolaterHelper::GetInterpolater(colorInterpolationMethod);
	m_colorInterpolationMethod = colorInterpolationMethod;
	m_isColorInterpolaterChanged = true;
}

void BaseParticleSpawnProperty::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_baseParticleSpawnPropertyGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_baseParticleSpawnPropertyCPU));
	m_baseParticleSpawnPropertyGPU->InitializeBuffer(device);
}

void BaseParticleSpawnProperty::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	if (m_isSpeedInterpolaterChanged)
	{
		//m_speedXInterpolater->GetCoefficients(m_speedXControlPoints);
		//m_speedYInterpolater->GetCoefficients(m_speedYControlPoints);
	}
	if (m_isColorInterpolaterChanged)
	{
		//m_colorRInterpolater->GetCoefficients(m_colorRControlPoints);
		//m_colorGInterpolater->GetCoefficients(m_colorGControlPoints);
		//m_colorBInterpolater->GetCoefficients(m_colorBControlPoints);
	}

	if (m_isParticleSpawnPropertyChanged)
	{
		m_baseParticleSpawnPropertyCPU.minMaxSpeed = XMFLOAT2(
			m_speedXInterpolater->GetInterpolated(m_emitterCurrentTime),
			m_speedYInterpolater->GetInterpolated(m_emitterCurrentTime)
		);
		m_baseParticleSpawnPropertyCPU.color = XMFLOAT3(
			m_colorRInterpolater->GetInterpolated(m_emitterCurrentTime),
			m_colorGInterpolater->GetInterpolated(m_emitterCurrentTime),
			m_colorBInterpolater->GetInterpolated(m_emitterCurrentTime)
		);

		m_baseParticleSpawnPropertyGPU->Stage(deviceContext);
		m_baseParticleSpawnPropertyGPU->Upload(deviceContext);

		m_isParticleSpawnPropertyChanged = false;
	}
}

void BaseParticleSpawnProperty::DrawPropertyUI()
{
}


