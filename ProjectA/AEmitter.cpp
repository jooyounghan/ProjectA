#include "AEmitter.h"

using namespace std;
using namespace DirectX;

AEmitter::AEmitter(
	UINT emitterType, 
	UINT emitterID,  
	bool& isEmitterWorldTransformChanged, 
	XMMATRIX& emitterWorldTransformRef, 
	bool& isEmitterForceChanged,
	SEmitterForceProperty& emitterForceRef,
	const XMVECTOR& position, 
	const XMVECTOR& angle
)
	: m_isEmitterWorldTransformChangedRef(isEmitterWorldTransformChanged),
	m_emitterWorldTransformRef(emitterWorldTransformRef),
	m_isThisWorldTransformChanged(false),
	m_position(position),
	m_angle(angle)
{

}

AEmitter::InjectAEmitterSpawnProperty(unique_ptr<BaseEmitterSpawnProperty> emitterSpawnProperty) noexcept 
{ 
	m_emitterSpawnProperty = std::move(emitterSpawnProperty); 
}

AEmitter::InjectAEmitterUpdateProperty(unique_ptr<BaseEmitterUpdateProperty> emitterUpdateProperty) noexcept 
{ 
	m_emitterUpdateProperty = std::move(emitterUpdateProperty); 
	m_emitterUpdateProperty->SetEmitterCurrentTime(&m_currnetEmitter);
}

AEmitter::InjectAParticleSpawnProperty(unique_ptr<BaseParticleSpawnProperty> particleSpawnProperty) noexcept 
{ 
	m_particleSpawnProperty = std::move(particleSpawnProperty); 
	m_particleSpawnProperty->SetEmitterCurrentTime(&m_currnetEmitter);
}

AEmitter::InjectAParticleUpdateProperty(unique_ptr<BaseParticleUpdateProperty> particleSpawnProperty) noexcept
{ 
	m_particleUpdateProperty = std::move(particleSpawnProperty); 
	m_particleUpdateProperty->SetEmitterForceProperty(&m_isEmitterForceChanged, &m_emitterForceRef);
}

void AEmitter::SetPosition(const XMVECTOR& position) noexcept
{
	m_position = position;
	m_isThisWorldTransformChanged = true;
}

void AEmitter::SetAngle(const XMVECTOR& angle) noexcept
{
	m_angle = angle;
	m_isThisWorldTransformChanged = true;
}

#define INITIALIZE_PROPRTY(PROPERTY, D, DC) if (PROPERTY) PROPERTY->Initialize(D, DC);

void AEmitter::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	INITIALIZE_PROPRTY(m_emitterSpawnProperty, device, deviceContext);
	INITIALIZE_PROPRTY(m_emitterUpdateProperty, device, deviceContext);
	INITIALIZE_PROPRTY(m_particleSpawnProperty, device, deviceContext);
	INITIALIZE_PROPRTY(m_particleUpdateProperty, device, deviceContext);
}

#define UPDATE_PROPRTY(PROPERTY, DC, DT) if (PROPERTY) PROPERTY->Update(DC, DT);

void AEmitter::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	if (m_isThisWorldTransformChanged)
	{
		m_emitterWorldTransformRef = XMMatrixAffineTransformation(
			XMVectorSet(1.f, 1.f, 1.f, 0.f),
			XMQuaternionIdentity(),
			XMQuaternionRotationRollPitchYawFromVector(m_angle),
			m_position
		);

		m_isThisWorldTransformChanged = false;
		m_isEmitterWorldTransformChangedRef = true;
	}
	UPDATE_PROPRTY(m_emitterSpawnProperty, deviceContext, dt);
	UPDATE_PROPRTY(m_emitterUpdateProperty, deviceContext, dt);
	UPDATE_PROPRTY(m_particleSpawnProperty, deviceContext, dt);
	UPDATE_PROPRTY(m_particleUpdateProperty, deviceContext, dt);
}
