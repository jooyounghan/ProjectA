#include "BaseParticleSpawnProperty.h"

#include "MacroUtilities.h"
#include "BufferMacroUtilities.h"

#include "imgui.h"

using namespace std;
using namespace D3D11;
using namespace DirectX;
using namespace ImGui;

BaseParticleSpawnProperty::BaseParticleSpawnProperty(
	const float& emitterDeltaTimeRef,
	const XMFLOAT2& minMaxLifeTime, 
	const XMFLOAT2& minEmitRadian, 
	const XMFLOAT2& maxEmitRadian, 
	EInterpolationMethod speedInterpolationMethod /*= EInterpolationMethod::Linear*/, 
	const vector<SControlPoint2>& speedControlPoints /*= vector<SSpeedControlPoint>()*/,
	EInterpolationMethod colorInterpolationMethod /*= EInterpolationMethod::Linear*/, 
	const vector<SControlPoint3>& colorControlPoints /*= vector<SColorControlPoint>() */
)
	: m_emitterDeltaTimeRef(emitterDeltaTimeRef),
	m_speedControlPoints(speedControlPoints),
	m_speedInterpolationMethod(speedInterpolationMethod),
	m_speedInterpolater(InterpolateHelper::GetPoint2Interpolater(speedInterpolationMethod)),
	m_colorControlPoints(colorControlPoints),
	m_colorInterpolationMethod(colorInterpolationMethod),
	m_colorInterpolater(InterpolateHelper::GetPoint3Interpolater(colorInterpolationMethod))
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

void BaseParticleSpawnProperty::SetSpeedControlPoints(const vector<SControlPoint2>& speedControlPoints)
{
	m_speedControlPoints = speedControlPoints;
	m_isParticleSpawnPropertyChanged = true;
}

void BaseParticleSpawnProperty::SetSpeedInterpolationMethod(EInterpolationMethod speedInterpolationMethod)
{
	m_speedInterpolater = InterpolateHelper::GetPoint2Interpolater(speedInterpolationMethod);
	m_speedInterpolationMethod = speedInterpolationMethod;
}

void BaseParticleSpawnProperty::SetColorControlPoints(const vector<SControlPoint3>& colorControlPoints)
{
	m_colorControlPoints = colorControlPoints;
	m_isParticleSpawnPropertyChanged = true;
}

void BaseParticleSpawnProperty::SetColorInterpolationMethod(EInterpolationMethod colorInterpolationMethod)
{
	m_colorInterpolater = InterpolateHelper::GetPoint3Interpolater(colorInterpolationMethod);
	m_colorInterpolationMethod = colorInterpolationMethod;
}

void BaseParticleSpawnProperty::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_baseParticleSpawnPropertyGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_baseParticleSpawnPropertyCPU));
	m_baseParticleSpawnPropertyGPU->InitializeBuffer(device);
}

void BaseParticleSpawnProperty::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	if (m_isParticleSpawnPropertyChanged)
	{
		m_baseParticleSpawnPropertyCPU.minMaxSpeed = m_speedInterpolater(m_speedControlPoints, m_emitterDeltaTimeRef);
		m_baseParticleSpawnPropertyCPU.color = m_colorInterpolater(m_colorControlPoints, m_emitterDeltaTimeRef);

		m_baseParticleSpawnPropertyGPU->Stage(deviceContext);
		m_baseParticleSpawnPropertyGPU->Upload(deviceContext);

		m_isParticleSpawnPropertyChanged = false;
	}
}

void BaseParticleSpawnProperty::DrawPropertyUI()
{
}

std::unique_ptr<BaseParticleSpawnProperty> BaseParticleSpawnProperty::DrawPropertyCreator()
{
	SeparatorText("파티클 생성 프로퍼티");
	return nullptr;
}

