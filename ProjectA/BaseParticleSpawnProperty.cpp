#include "BaseParticleSpawnProperty.h"

#include "MacroUtilities.h"
#include "BufferMacroUtilities.h"

#include "imgui.h"

using namespace std;
using namespace D3D11;
using namespace DirectX;
using namespace ImGui;

#define CreateSpeedInterpolater()									\
m_speedInterpolater = InterpolationSelector::CreateInterpolater<2>(	\
	m_speedInterpolationMethod,										\
	m_speedInitControlPoint, m_speedFinalControlPoint,				\
	m_speedControlPoints											\
)																	\

#define CreateColorInterpolater()									\
m_colorInterpolater = InterpolationSelector::CreateInterpolater<3>(	\
	m_colorInterpolationMethod,										\
	m_colorInitControlPoint, m_colorFinalControlPoint,				\
	m_colorControlPoints											\
)					

BaseParticleSpawnProperty::BaseParticleSpawnProperty(float& emitterCurrentTime)
	: m_emitterCurrentTime(emitterCurrentTime)
{
	AutoZeroMemory(m_baseParticleSpawnPropertyCPU);

}

void BaseParticleSpawnProperty::SetMinMaxLifeTime(const DirectX::XMFLOAT2& minMaxLifeTime)
{
	m_baseParticleSpawnPropertyCPU.minMaxLifeTime = minMaxLifeTime;
}

void BaseParticleSpawnProperty::SetMinEmitRadian(const XMFLOAT2& minEmitRadian)
{
	m_baseParticleSpawnPropertyCPU.minEmitRadian = minEmitRadian;
}

void BaseParticleSpawnProperty::SetMaxEmitRadian(const XMFLOAT2& maxEmitRadian)
{
	m_baseParticleSpawnPropertyCPU.maxEmitRadian = maxEmitRadian;
}

void BaseParticleSpawnProperty::SetInitSpeed(const SControlPoint<2>& speed) noexcept
{
	m_speedInitControlPoint = speed;
	CreateSpeedInterpolater();
}

void BaseParticleSpawnProperty::SetFinalSpeed(const SControlPoint<2>& speed) noexcept
{
	m_speedFinalControlPoint = speed;
	CreateSpeedInterpolater();
}

void BaseParticleSpawnProperty::SetSpeedControlPoints(const std::vector<SControlPoint<2>>& speedControlPoints)
{
	m_speedControlPoints = speedControlPoints;
	CreateSpeedInterpolater();
}

void BaseParticleSpawnProperty::SetSpeedInterpolationMethod(EInterpolationMethod speedInterpolationMethod)
{
	m_speedInterpolationMethod = speedInterpolationMethod;
	CreateSpeedInterpolater();
}

void BaseParticleSpawnProperty::SetInitColor(const SControlPoint<3>& color) noexcept
{
	m_colorInitControlPoint = color;
	CreateColorInterpolater();
}

void BaseParticleSpawnProperty::SetFinalColor(const SControlPoint<3>& color) noexcept
{
	m_colorFinalControlPoint = color;
	CreateColorInterpolater();
}

void BaseParticleSpawnProperty::SetColorControlPoints(const std::vector<SControlPoint<3>>& colorControlPoints)
{
	m_colorControlPoints = colorControlPoints;
	CreateColorInterpolater();
}

void BaseParticleSpawnProperty::SetColorInterpolationMethod(EInterpolationMethod colorInterpolationMethod)
{
	m_colorInterpolationMethod = colorInterpolationMethod;
	CreateColorInterpolater();
}

void BaseParticleSpawnProperty::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_baseParticleSpawnPropertyGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_baseParticleSpawnPropertyCPU));
	m_baseParticleSpawnPropertyGPU->InitializeBuffer(device);
}

void BaseParticleSpawnProperty::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	//m_baseParticleSpawnPropertyCPU.minMaxSpeed = XMFLOAT2(
	//	m_speedXInterpolater->GetInterpolated(m_emitterCurrentTime),
	//	m_speedYInterpolater->GetInterpolated(m_emitterCurrentTime)
	//);
	//m_baseParticleSpawnPropertyCPU.color = XMFLOAT3(
	//	m_colorRInterpolater->GetInterpolated(m_emitterCurrentTime),
	//	m_colorGInterpolater->GetInterpolated(m_emitterCurrentTime),
	//	m_colorBInterpolater->GetInterpolated(m_emitterCurrentTime)
	//);

	m_baseParticleSpawnPropertyGPU->Stage(deviceContext);
	m_baseParticleSpawnPropertyGPU->Upload(deviceContext);
}

void BaseParticleSpawnProperty::DrawPropertyUI()
{
}


