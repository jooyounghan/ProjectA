#include "ParticleEmitter.h"

#include "MacroUtilities.h"
#include "BufferMacroUtilities.h"

using namespace std;
using namespace DirectX;
using namespace D3D11;

CParticleEmitter::CParticleEmitter(
	UINT emitterID,
	bool& isEmitterWorldTransformChanged,
	XMMATRIX& emitterWorldTransform,
	const XMVECTOR& position, 
	const XMVECTOR& angle, 
	const XMVECTOR& emitVelocity
)
	: m_position(position), m_angle(angle),
	m_isEmitterWorldTransformChanged(isEmitterWorldTransformChanged),
	m_emitterWorldTransform(emitterWorldTransform),
	m_isEmitterPropertiesChanged(false)
{
	AutoZeroMemory(m_emitterPropertiesCPU);
	SetEmitVelocity(emitVelocity);
	SetEmitterID(emitterID);
	m_emitterPropertiesCPU.emitterWorldTransform = m_emitterWorldTransform;


	m_isThisWorldTransformChanged = true;
	m_isEmitterPropertiesChanged = true;
}


void CParticleEmitter::SetPosition(const DirectX::XMVECTOR& position) noexcept
{
	m_position = position;
	m_isThisWorldTransformChanged = true;
}


void CParticleEmitter::SetAngle(const DirectX::XMVECTOR& angle) noexcept
{
	m_angle = angle;
	m_isThisWorldTransformChanged = true;
}

void CParticleEmitter::SetEmitVelocity(const DirectX::XMVECTOR& emitVelocity) noexcept
{
	XMStoreFloat3(&m_emitterPropertiesCPU.emitVelocity, emitVelocity);
	m_isEmitterPropertiesChanged = true;
}

void CParticleEmitter::SetEmitterID(UINT emitterID) noexcept
{
	m_emitterPropertiesCPU.emitterID = emitterID;
	m_isEmitterPropertiesChanged = true;
}

void CParticleEmitter::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_emitterPropertiesGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_emitterPropertiesCPU));
	m_emitterPropertiesGPU->InitializeBuffer(device);
}

void CParticleEmitter::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	//if (m_isThisWorldTransformChanged)
	//{
	m_angle = XMVectorAddAngles(m_angle, XMVectorSet(0.f, 5.f * dt, 0.f, 0.f));
		m_emitterWorldTransform = XMMatrixAffineTransformation(
			XMVectorSet(1.f, 1.f, 1.f, 0.f),
			XMQuaternionIdentity(),
			XMQuaternionRotationRollPitchYawFromVector(m_angle),
			m_position
		);
		m_emitterPropertiesCPU.emitterWorldTransform = XMMatrixTranspose(m_emitterWorldTransform);
		m_isThisWorldTransformChanged = false;
		m_isEmitterWorldTransformChanged = true;
		m_isEmitterPropertiesChanged = true;
	//}

	if (m_isEmitterPropertiesChanged)
	{
		m_emitterPropertiesGPU->Stage(deviceContext);
		m_emitterPropertiesGPU->Upload(deviceContext);
		m_isEmitterPropertiesChanged = false;
	}
}
