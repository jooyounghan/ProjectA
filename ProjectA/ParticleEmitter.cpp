#include "ParticleEmitter.h"

#include "MacroUtilities.h"
#include "BufferMacroUtilities.h"

using namespace std;
using namespace DirectX;
using namespace D3D11;

CParticleEmitter::CParticleEmitter(
	UINT emitterID,
	bool& isEmitterWorldTransformChanged,
	DirectX::XMMATRIX& emitterWorldTransform,
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
	m_emitterPropertiesCPU.emitterID = emitterID;
	SetEmitVelocity(emitVelocity);

	m_isEmitterWorldTransformChanged = true;
	m_isThisWorldTransformChanged = true;
	m_isEmitterPropertiesChanged = true;
}


void CParticleEmitter::SetPosition(const DirectX::XMVECTOR& position) noexcept
{
	m_position = position;
	m_isEmitterWorldTransformChanged = true;
	m_isThisWorldTransformChanged = true;
}


void CParticleEmitter::SetAngle(const DirectX::XMVECTOR& angle) noexcept
{
	m_angle = angle;
	m_isEmitterWorldTransformChanged = true;
	m_isThisWorldTransformChanged = true;
}

void CParticleEmitter::SetEmitVelocity(const DirectX::XMVECTOR& emitVelocity) noexcept
{
	m_emitterPropertiesCPU.emitVelocity = emitVelocity;
	m_isEmitterPropertiesChanged = true;
}

void CParticleEmitter::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_emittorPropertiesGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_emitterPropertiesCPU));
	m_emittorPropertiesGPU->InitializeBuffer(device);
}

void CParticleEmitter::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	if (m_isThisWorldTransformChanged)
	{
		m_emitterWorldTransform = XMMatrixAffineTransformation(
			XMVectorSet(1.f, 1.f, 1.f, 0.f),
			XMQuaternionIdentity(),
			XMQuaternionRotationRollPitchYawFromVector(m_angle),
			m_position
		);
		m_isThisWorldTransformChanged = false;
	}

	if (m_isEmitterPropertiesChanged)
	{
		m_emittorPropertiesGPU->Stage(deviceContext);
		m_emittorPropertiesGPU->Upload(deviceContext);
		m_isEmitterPropertiesChanged = false;
	}
}
