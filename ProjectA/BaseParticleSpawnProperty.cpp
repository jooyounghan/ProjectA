#include "BaseParticleSpawnProperty.h"

#include "MacroUtilities.h"
#include "BufferMacroUtilities.h"

#include "imgui.h"

using namespace std;
using namespace D3D11;
using namespace DirectX;
using namespace ImGui;

BaseParticleSpawnProperty::BaseParticleSpawnProperty(
	const XMFLOAT2& minMaxLifeTime, 
	const XMFLOAT2& minEmitRadian, 
	const XMFLOAT2& maxEmitRadian, 
	EInterpolationMethod speedInterpolationMethod /*= EInterpolationMethod::Linear*/, 
	const std::vector<SControlPoint>& speedXControlPoints/* = std::vector<SControlPoint>()*/,
	const std::vector<SControlPoint>& speedYControlPoints/* = std::vector<SControlPoint>()*/,
	EInterpolationMethod colorInterpolationMethod /*= EInterpolationMethod::Linear*/,
	const std::vector<SControlPoint>& colorRControlPoints/* = std::vector<SControlPoint>()*/,
	const std::vector<SControlPoint>& colorGControlPoints/* = std::vector<SControlPoint>()*/,
	const std::vector<SControlPoint>& colorBControlPoints/* = std::vector<SControlPoint>()*/
)
	: m_speedXControlPoints(speedXControlPoints),
	m_speedYControlPoints(speedYControlPoints),
	m_speedInterpolationMethod(speedInterpolationMethod),
	m_speedXInterpolater(InterpolaterHelper::GetInterpolater(speedInterpolationMethod)),
	m_speedYInterpolater(InterpolaterHelper::GetInterpolater(speedInterpolationMethod)),
	m_colorRControlPoints(colorRControlPoints),
	m_colorGControlPoints(colorGControlPoints),
	m_colorBControlPoints(colorBControlPoints),
	m_colorInterpolationMethod(colorInterpolationMethod),
	m_colorRInterpolater(InterpolaterHelper::GetInterpolater(colorInterpolationMethod)),
	m_colorGInterpolater(InterpolaterHelper::GetInterpolater(colorInterpolationMethod)),
	m_colorBInterpolater(InterpolaterHelper::GetInterpolater(colorInterpolationMethod))
{
	AutoZeroMemory(m_baseParticleSpawnPropertyCPU);
	m_baseParticleSpawnPropertyCPU.minMaxLifeTime = minMaxLifeTime;
	m_baseParticleSpawnPropertyCPU.minEmitRadian = minEmitRadian;
	m_baseParticleSpawnPropertyCPU.maxEmitRadian = maxEmitRadian;
	m_isParticleSpawnPropertyChanged = true;
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

void BaseParticleSpawnProperty::SetSpeedControlPoints(
	const vector<SControlPoint>& speedXControlPoints, 
	const vector<SControlPoint>& speedYControlPoints
)
{
	m_speedXControlPoints = speedXControlPoints;
	m_speedYControlPoints = speedYControlPoints;
	m_isParticleSpawnPropertyChanged = true;
	m_isSpeedInterpolaterChanged = true;
}

void BaseParticleSpawnProperty::SetSpeedInterpolationMethod(EInterpolationMethod speedInterpolationMethod)
{
	m_speedXInterpolater = InterpolaterHelper::GetInterpolater(speedInterpolationMethod);
	m_speedYInterpolater = InterpolaterHelper::GetInterpolater(speedInterpolationMethod);
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
	m_colorRInterpolater = InterpolaterHelper::GetInterpolater(colorInterpolationMethod);
	m_colorGInterpolater = InterpolaterHelper::GetInterpolater(colorInterpolationMethod);
	m_colorBInterpolater = InterpolaterHelper::GetInterpolater(colorInterpolationMethod);
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
		m_speedXInterpolater->GetCoefficients(m_speedXControlPoints);
		m_speedYInterpolater->GetCoefficients(m_speedYControlPoints);
	}
	if (m_isColorInterpolaterChanged)
	{
		m_colorRInterpolater->GetCoefficients(m_colorRControlPoints);
		m_colorGInterpolater->GetCoefficients(m_colorGControlPoints);
		m_colorBInterpolater->GetCoefficients(m_colorBControlPoints);
	}

	if (m_isParticleSpawnPropertyChanged)
	{
		const float& emitterCurrentTime = *m_emitterCurrentTime;
		m_baseParticleSpawnPropertyCPU.minMaxSpeed = XMFLOAT2(
			m_speedXInterpolater->GetInterpolated(emitterCurrentTime),
			m_speedYInterpolater->GetInterpolated(emitterCurrentTime)
		);
		m_baseParticleSpawnPropertyCPU.color = XMFLOAT3(
			m_colorRInterpolater->GetInterpolated(emitterCurrentTime),
			m_colorGInterpolater->GetInterpolated(emitterCurrentTime),
			m_colorBInterpolater->GetInterpolated(emitterCurrentTime)
		);

		m_baseParticleSpawnPropertyGPU->Stage(deviceContext);
		m_baseParticleSpawnPropertyGPU->Upload(deviceContext);

		m_isParticleSpawnPropertyChanged = false;
	}
}

void BaseParticleSpawnProperty::DrawPropertyUI()
{
}

std::unique_ptr<BaseParticleSpawnProperty> BaseParticleSpawnProperty::DrawPropertyCreator(bool& isApplied)
{
	SeparatorText("파티클 생성 프로퍼티");
	return nullptr;
}

