#include "ParticleEmitter.h"
#include "BufferMacroUtilities.h"
#include "ModelFactory.h"

using namespace std;
using namespace DirectX;
using namespace D3D11;

const vector<XMFLOAT3> CParticleEmitter::GEmitterBoxPositions = ModelFactory::CreateBoxPositions(XMVectorSet(1.f, 1.f, 1.f, 0.f));
const vector<UINT> CParticleEmitter::GEmitterBoxIndices = ModelFactory::CreateIndices();

CParticleEmitter::CParticleEmitter(
	const XMVECTOR& position, 
	const XMVECTOR& angle, 
	const XMVECTOR& emitVelocity
)
	: m_position(position), m_angle(angle),
	m_emitterPropertiesGPU(PASS_SINGLE(m_emitterPropertiesCPU)),
	m_isEmitterPropertiesChanged(false),
	m_positionBuffer(12, static_cast<UINT>(GEmitterBoxPositions.size()), GEmitterBoxPositions.data(), D3D11_BIND_VERTEX_BUFFER),
	m_indexBuffer(4, static_cast<UINT>(GEmitterBoxIndices.size()), GEmitterBoxIndices.data(), D3D11_BIND_VERTEX_BUFFER)
{
	SetEmitVelocity(emitVelocity);

	vector<XMFLOAT3> boxPositions = ModelFactory::CreateBoxPositions(XMVectorSet(1.f, 1.f, 1.f, 0.f));
	vector<UINT> boxIndices = ModelFactory::CreateIndices();
}

void CParticleEmitter::SetEmitVelocity(const DirectX::XMVECTOR& emitVelocity) noexcept
{
	m_emitterPropertiesCPU.emitVelocity = emitVelocity;
	m_isEmitterPropertiesChanged = true;
}

void CParticleEmitter::SetPosition(const DirectX::XMVECTOR& position) noexcept
{
	m_position = position;
	m_isEmitterPropertiesChanged = true;
}

void CParticleEmitter::SetAngle(const DirectX::XMVECTOR& angle) noexcept
{
	m_angle = angle;
	m_isEmitterPropertiesChanged = true;
}

void CParticleEmitter::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_positionBuffer.InitializeBuffer(device);
	m_indexBuffer.InitializeBuffer(device);
	m_emitterPropertiesGPU.InitializeBuffer(device);
}

void CParticleEmitter::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	if (m_isEmitterPropertiesChanged)
	{
		m_emitterPropertiesCPU.toWorldTransform = XMMatrixAffineTransformation(
			XMVectorSet(1.f, 1.f, 1.f, 0.f),
			XMQuaternionIdentity(),
			XMQuaternionRotationRollPitchYaw(
				XMConvertToRadians(XMVectorGetX(m_angle)),
				XMConvertToRadians(XMVectorGetY(m_angle)),
				XMConvertToRadians(XMVectorGetZ(m_angle))
			),
			m_position
		);

		m_emitterPropertiesGPU.Stage(deviceContext);
		m_emitterPropertiesGPU.Upload(deviceContext);
	}
}
