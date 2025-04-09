#include "ParticleEmitter.h"

#include "MacroUtilities.h"
#include "BufferMacroUtilities.h"

#include <cmath>

using namespace std;
using namespace DirectX;
using namespace D3D11;

CParticleEmitter::CParticleEmitter(
	UINT emitterID,
	UINT emitterType,
	const vector<SEmitTimeRate>& emitProfiles,
	bool& isEmitterWorldTransformChanged,
	XMMATRIX& emitterWorldTransform,
	const XMVECTOR& position, 
	const XMVECTOR& angle, 
	const XMVECTOR& emitVelocity
)
	: m_position(position), m_angle(angle),
	m_isEmitterWorldTransformChanged(isEmitterWorldTransformChanged),
	m_emitterWorldTransform(emitterWorldTransform),
	m_isEmitterPropertiesChanged(false),
	m_totalPlayTime(0.f), m_currentEmitCount(0),
	m_emitRateProfiles(emitProfiles)
{
	AutoZeroMemory(m_emitterPropertiesCPU);
	m_emitterPropertiesCPU.emitterID = emitterID;
	m_emitterPropertiesCPU.emitterType = emitterType;
	m_emitterPropertiesCPU.emitterWorldTransform = m_emitterWorldTransform;
	SetEmitVelocity(emitVelocity);

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

void CParticleEmitter::UpdateCurrentEmitCount()
{
	if (m_emitRateProfiles.empty())
	{
		m_currentEmitCount = 0;
		return;
	}

	size_t emitProfileCounts = m_emitRateProfiles.size();
	for (size_t idx = 0; idx < emitProfileCounts - 1; ++idx)
	{
		float emitTimeFrom = m_emitRateProfiles[idx].time;
		float emitTimeTo = m_emitRateProfiles[idx + 1].time;

		if (emitTimeFrom - 1E-3f < m_totalPlayTime && m_totalPlayTime < emitTimeTo + 1E-3f)
		{
			int emitCountFrom = m_emitRateProfiles[idx].emitCount;
			int emitCountTo = m_emitRateProfiles[idx + 1].emitCount;

			m_currentEmitCount =
				static_cast<UINT>(max(0.f, emitCountFrom + (emitCountTo - emitCountFrom) * (m_totalPlayTime - emitTimeFrom) / (emitTimeTo - emitTimeFrom)));
			printf("%d\n", m_currentEmitCount);
			return;
		}
	}

	if (m_totalPlayTime < m_emitRateProfiles.front().time) m_currentEmitCount = m_emitRateProfiles.front().emitCount;
	if (m_totalPlayTime > m_emitRateProfiles.back().time) m_currentEmitCount = m_emitRateProfiles.back().emitCount;
	printf("%d\n", m_currentEmitCount);
}

void CParticleEmitter::SetEmitVelocity(const DirectX::XMVECTOR& emitVelocity) noexcept
{
	XMStoreFloat3(&m_emitterPropertiesCPU.emitVelocity, emitVelocity);
	m_isEmitterPropertiesChanged = true;
}


void CParticleEmitter::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_emitterPropertiesGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_emitterPropertiesCPU));
	m_emitterPropertiesGPU->InitializeBuffer(device);
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
			m_emitterPropertiesCPU.emitterWorldTransform = XMMatrixTranspose(m_emitterWorldTransform);
			m_isThisWorldTransformChanged = false;
			m_isEmitterWorldTransformChanged = true;
			m_isEmitterPropertiesChanged = true;
	}

	if (m_isEmitterPropertiesChanged)
	{
		m_emitterPropertiesGPU->Stage(deviceContext);
		m_emitterPropertiesGPU->Upload(deviceContext);
		m_isEmitterPropertiesChanged = false;
	}

	m_totalPlayTime += dt;
	UpdateCurrentEmitCount();
}
