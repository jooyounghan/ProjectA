#include "AEmitter.h"
#include "MacroUtilities.h"
#include "BufferMacroUtilities.h"

#include <exception>

using namespace std;
using namespace DirectX;
using namespace D3D11;

UINT AEmitter::GEmitterMaxCount = 0;
queue<UINT> AEmitter::GEmitterIDQueue;
vector<DirectX::XMMATRIX> AEmitter::GEmitterWorldTransformCPU;
unique_ptr<D3D11::CDynamicBuffer> AEmitter::GEmitterWorldTransformGPU = nullptr;
vector<SEmitterForceProperty> AEmitter::GEmitterForcePropertyCPU;
unique_ptr<D3D11::CStructuredBuffer> AEmitter::GEmitterForcePropertyGPU = nullptr;
bool AEmitter::GIsEmitterWorldPositionChanged = false;
bool AEmitter::GIsEmitterForceChanged = false;

void AEmitter::InitializeGlobalEmitterProperty(UINT emitterMaxCount)
{
	GEmitterMaxCount = emitterMaxCount;
	for (UINT idx = 0; idx < GEmitterMaxCount; ++idx)
	{
		GEmitterIDQueue.push(idx);
	}

	GIsEmitterWorldPositionChanged = false;
	GIsEmitterForceChanged = false;

	SEmitterForceProperty initialForceProperty;
	AutoZeroMemory(initialForceProperty);
	GEmitterWorldTransformCPU.resize(GEmitterMaxCount, XMMatrixIdentity());
	GEmitterForcePropertyCPU.resize(GEmitterMaxCount, initialForceProperty);

	GEmitterWorldTransformGPU = make_unique<CDynamicBuffer>(
		static_cast<UINT>(sizeof(XMMATRIX)),
		static_cast<UINT>(GEmitterWorldTransformCPU.size()),
		GEmitterWorldTransformCPU.data(),
		D3D11_BIND_VERTEX_BUFFER
	);
	GEmitterForcePropertyGPU = make_unique<CStructuredBuffer>(
		static_cast<UINT>(sizeof(CStructuredBuffer)),
		static_cast<UINT>(GEmitterForcePropertyCPU.size()),
		GEmitterForcePropertyCPU.data()
	);
}

void AEmitter::UpdateGlobalEmitterProperty(ID3D11DeviceContext* deviceContext)
{
	if (GIsEmitterWorldPositionChanged)
	{
		GEmitterWorldTransformGPU->Stage(deviceContext);
		GEmitterWorldTransformGPU->Upload(deviceContext);
	}

	if (GIsEmitterForceChanged)
	{
		GEmitterForcePropertyGPU->Stage(deviceContext);
		GEmitterForcePropertyGPU->Upload(deviceContext);
	}
}

UINT AEmitter::IssueAvailableEmitterID()
{
	if (GEmitterIDQueue.empty()) { throw exception("No Emitter ID To Issue"); }

	UINT emitterID = GEmitterIDQueue.front();
	GEmitterIDQueue.pop();

	return emitterID;
}

AEmitter::AEmitter(
	UINT emitterType, 
	UINT emitterID,  
	bool& isEmitterWorldTransformChanged, 
	XMMATRIX& emitterWorldTransform, 
	bool& isEmitterForceChanged,
	SEmitterForceProperty& emitterForce,
	const XMVECTOR& position, 
	const XMVECTOR& angle
)
	: m_isEmitterWorldTransformChanged(isEmitterWorldTransformChanged),
	m_emitterWorldTransform(emitterWorldTransform),
	m_isEmitterForceChanged(isEmitterForceChanged),
	m_emitterForce(emitterForce),
	m_isThisWorldTransformChanged(false),
	m_position(position),
	m_angle(angle)
{

}

void AEmitter::InjectAEmitterSpawnProperty(unique_ptr<BaseEmitterSpawnProperty>& emitterSpawnProperty) noexcept 
{ 
	m_emitterSpawnProperty = std::move(emitterSpawnProperty); 
}

void AEmitter::InjectAEmitterUpdateProperty(unique_ptr<BaseEmitterUpdateProperty>& emitterUpdateProperty) noexcept
{ 
	m_emitterUpdateProperty = std::move(emitterUpdateProperty); 
}

void AEmitter::InjectAParticleSpawnProperty(unique_ptr<BaseParticleSpawnProperty>& particleSpawnProperty) noexcept
{ 
	m_particleSpawnProperty = std::move(particleSpawnProperty); 
}

void AEmitter::InjectAParticleUpdateProperty(unique_ptr<BaseParticleUpdateProperty>& particleSpawnProperty) noexcept
{ 
	m_particleUpdateProperty = std::move(particleSpawnProperty); 
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
		m_emitterWorldTransform = XMMatrixAffineTransformation(
			XMVectorSet(1.f, 1.f, 1.f, 0.f),
			XMQuaternionIdentity(),
			XMQuaternionRotationRollPitchYawFromVector(m_angle),
			m_position
		);

		m_isThisWorldTransformChanged = false;
		m_isEmitterWorldTransformChanged = true;
	}
	UPDATE_PROPRTY(m_emitterSpawnProperty, deviceContext, dt);
	UPDATE_PROPRTY(m_emitterUpdateProperty, deviceContext, dt);
	UPDATE_PROPRTY(m_particleSpawnProperty, deviceContext, dt);
	UPDATE_PROPRTY(m_particleUpdateProperty, deviceContext, dt);
}
