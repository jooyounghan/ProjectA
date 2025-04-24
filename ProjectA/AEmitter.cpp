#include "AEmitter.h"
#include "MacroUtilities.h"
#include "BufferMacroUtilities.h"

#include "DynamicBuffer.h"

#include "BaseEmitterSpawnProperty.h"
#include "BaseEmitterUpdateProperty.h"
#include "BaseParticleSpawnProperty.h"
#include "BaseParticleUpdateProperty.h"

#include <exception>

using namespace std;
using namespace DirectX;
using namespace D3D11;

AEmitter::AEmitter(
	UINT emitterType,
	UINT emitterID,
	const XMVECTOR& position,
	const XMVECTOR& angle
) :
	m_isEmitterPropertyChanged(false),
	m_isSpawned(false),
	m_emitterCurrentTime(0.f),
	m_emitterLoopTime(10.f),
	m_position(position),
	m_angle(angle)
{
	AutoZeroMemory(m_emitterPropertyCPU);
	m_emitterPropertyCPU.emitterType = emitterType;
	m_emitterPropertyCPU.emitterID = emitterID;
	m_isEmitterPropertyChanged = true;
}

ID3D11Buffer* AEmitter::GetEmitterPropertyBuffer() const noexcept { return m_emitterPropertyGPU->GetBuffer(); }

void AEmitter::SetEmitterForceProperty(const SEmitterForceProperty& emitterForce)
{
	const UINT& emitterID = m_emitterPropertyCPU.emitterID;
	EmitterStaticData::AddChangedEmitterForceID(emitterID);
	EmitterStaticData::GEmitterForcePropertyCPU[emitterID] = emitterForce;
}

void AEmitter::SetInterpolaterLifeInformation(float life)
{
	const UINT& emitterID = m_emitterPropertyCPU.emitterID;
	EmitterStaticData::AddChangedEmitterInterpolaterInformationID(emitterID);
	SEmitterInterpolaterInformation& emitterInterpolaterInformation = EmitterStaticData::GEmitterInterpolaterInformationCPU[emitterID];
	emitterInterpolaterInformation.maxLife = life;
}

void AEmitter::SetColorInterpolaterInformation(UINT interpolaterID, UINT interpolaterDegree)
{
	const UINT& emitterID = m_emitterPropertyCPU.emitterID;
	EmitterStaticData::AddChangedEmitterInterpolaterInformationID(emitterID);
	SEmitterInterpolaterInformation& emitterInterpolaterInformation = EmitterStaticData::GEmitterInterpolaterInformationCPU[emitterID];
	emitterInterpolaterInformation.colorInterpolaterID = interpolaterID;
	emitterInterpolaterInformation.colorInterpolaterDegree = interpolaterDegree;
}

void AEmitter::InjectAEmitterSpawnProperty(unique_ptr<CBaseEmitterSpawnProperty>& emitterSpawnProperty) noexcept
{ 
	m_emitterSpawnProperty = std::move(emitterSpawnProperty); 
}

void AEmitter::InjectAEmitterUpdateProperty(unique_ptr<CBaseEmitterUpdateProperty>& emitterUpdateProperty) noexcept
{ 
	m_emitterUpdateProperty = std::move(emitterUpdateProperty); 
}

void AEmitter::InjectAParticleSpawnProperty(unique_ptr<CBaseParticleSpawnProperty>& particleSpawnProperty) noexcept
{ 
	m_particleSpawnProperty = std::move(particleSpawnProperty); 
}

void AEmitter::InjectAParticleUpdateProperty(unique_ptr<CBaseParticleUpdateProperty>& particleSpawnProperty) noexcept
{ 
	m_particleUpdateProperty = std::move(particleSpawnProperty); 
}

void AEmitter::SetPosition(const XMVECTOR& position) noexcept
{
	m_position = position;
	m_isEmitterPropertyChanged = true;
}

void AEmitter::SetAngle(const XMVECTOR& angle) noexcept
{
	m_angle = angle;
	m_isEmitterPropertyChanged = true;
}

#define INITIALIZE_PROPRTY(PROPERTY, D, DC) if (PROPERTY) PROPERTY->Initialize(D, DC);

void AEmitter::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_emitterPropertyGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_emitterPropertyCPU));
	m_emitterPropertyGPU->InitializeBuffer(device);

	INITIALIZE_PROPRTY(m_emitterSpawnProperty, device, deviceContext);
	INITIALIZE_PROPRTY(m_emitterUpdateProperty, device, deviceContext);
	INITIALIZE_PROPRTY(m_particleSpawnProperty, device, deviceContext);
	INITIALIZE_PROPRTY(m_particleUpdateProperty, device, deviceContext);
}

#define UPDATE_PROPRTY(PROPERTY, DC, DT) if (PROPERTY) PROPERTY->Update(DC, DT);

void AEmitter::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	if (m_isEmitterPropertyChanged)
	{
		const UINT& emitterID = m_emitterPropertyCPU.emitterID;
		EmitterStaticData::AddChangedEmitterWorldPositionID(emitterID);

		XMMATRIX& emitterWorldTrans = EmitterStaticData::GEmitterWorldTransformCPU[emitterID];
		emitterWorldTrans = XMMatrixAffineTransformation(
			XMVectorSet(1.f, 1.f, 1.f, 0.f),
			XMQuaternionIdentity(),
			XMQuaternionRotationRollPitchYawFromVector(m_angle),
			m_position
		);
		m_emitterPropertyCPU.emitterWorldTransform = XMMatrixTranspose(emitterWorldTrans);

		m_emitterPropertyGPU->Stage(deviceContext);
		m_emitterPropertyGPU->Upload(deviceContext);


		m_isEmitterPropertyChanged = false;
	}
	UPDATE_PROPRTY(m_emitterSpawnProperty, deviceContext, dt);
	UPDATE_PROPRTY(m_emitterUpdateProperty, deviceContext, dt);
	UPDATE_PROPRTY(m_particleSpawnProperty, deviceContext, dt);
	UPDATE_PROPRTY(m_particleUpdateProperty, deviceContext, dt);
}
