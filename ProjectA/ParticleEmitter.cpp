#include "ParticleEmitter.h"
#include "BufferMacroUtilities.h"

using namespace std;
using namespace DirectX;
using namespace D3D11;

CParticleEmitter::CParticleEmitter(
	UINT emitterID,
	UINT emitterType,
	float particleDensity,
	float particleRadius,
	bool& isEmitterWorldPositionChanged,
	bool& isEmitterWorldTransformChanged,
	XMVECTOR& positionRef,
	XMMATRIX& emitterWorldTransformRef,
	const XMVECTOR& position,
	const XMVECTOR& angle,
	const XMFLOAT2& minInitRadians,
	const XMFLOAT2& maxInitRadians,
	const XMFLOAT2& minMaxRadius,
	UINT initialParticleCount
)
	: m_angle(angle),
	m_isEmitterWorldPositionChanged(isEmitterWorldPositionChanged),
	m_isEmitterWorldTransformChanged(isEmitterWorldTransformChanged),
	m_positionRef(positionRef),
	emitterWorldTransformRef(emitterWorldTransformRef)
{
	m_positionRef = position;
	m_emitterPropertyCPU.emitterWorldTransform = XMMatrixIdentity();
	m_emitterPropertyCPU.emitterID = emitterID;
	m_emitterPropertyCPU.emitterType = emitterType;
	m_emitterPropertyCPU.particleDenstiy = particleDensity;
	m_emitterPropertyCPU.particleRadius = particleRadius;
	m_isThisWorldTransformChanged = true;

	m_emitterSpawnProperty = make_unique<CEmitterSpawnProperty>(
		minInitRadians, maxInitRadians, 
		minMaxRadius, initialParticleCount
);
	m_particleSpawnProperty = make_unique<CParticleSpawnProperty>(
		XMFLOAT2(0.f, 0.f), XMFLOAT2(XM_2PI, XM_2PI), XMFLOAT2(0.f, 0.f)
	);
}


void CParticleEmitter::SetParticleDensity(float particleDensity) 
{ 
	m_emitterPropertyCPU.particleDenstiy = particleDensity; 
	m_isEmitterPropertyChanged = true;
}

void CParticleEmitter::SetParticleRadius(float particleRadius)
{
	m_emitterPropertyCPU.particleRadius = particleRadius;
	m_isEmitterPropertyChanged = true;
}

void CParticleEmitter::SetPosition(const DirectX::XMVECTOR& position) noexcept
{
	m_positionRef = position;
	m_isThisWorldTransformChanged = true;
	m_isEmitterWorldPositionChanged = true;
}


void CParticleEmitter::SetAngle(const DirectX::XMVECTOR& angle) noexcept
{
	m_angle = angle;
	m_isThisWorldTransformChanged = true;
}


void CParticleEmitter::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_emitterSpawnProperty->Initialize(device, deviceContext);
	m_particleSpawnProperty->Initialize(device, deviceContext);

	m_emitterPropertyGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_emitterPropertyCPU));
	m_emitterPropertyGPU->InitializeBuffer(device);
}

void CParticleEmitter::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	if (m_isThisWorldTransformChanged)
	{
		emitterWorldTransformRef = XMMatrixAffineTransformation(
			XMVectorSet(1.f, 1.f, 1.f, 0.f),
			XMQuaternionIdentity(),
			XMQuaternionRotationRollPitchYawFromVector(m_angle),
			m_positionRef
		);
			
		m_isEmitterPropertyChanged = true;
		m_isThisWorldTransformChanged = false;
		m_isEmitterWorldTransformChanged = true;
	}

	if (m_isEmitterPropertyChanged)
	{
		m_emitterPropertyCPU.emitterWorldTransform = XMMatrixTranspose(emitterWorldTransformRef);
		m_emitterPropertyGPU->Stage(deviceContext);
		m_emitterPropertyGPU->Upload(deviceContext);
		m_isEmitterPropertyChanged = false;
	}

	m_particleSpawnProperty->Update(deviceContext, dt);
}
