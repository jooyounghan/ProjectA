#include "ParticleSpawnProperty.h"

#include "MacroUtilities.h"
#include "BufferMacroUtilities.h"

using namespace std;
using namespace D3D11;
using namespace DirectX;

CParticleSpawnProperty::CParticleSpawnProperty(
	const DirectX::XMFLOAT2& minEmitRadians,
	const DirectX::XMFLOAT2& maxEmitRadians,
	float emitSpeed,
	const vector<SEmitRate>& emitRateProfiles,
	bool isPlayLoop,
	float loopTime
)
	: m_emitRateProfiles(emitRateProfiles), m_isPlayLoop(isPlayLoop), m_loopTime(loopTime)
{
	AutoZeroMemory(m_particleSpawnPropertyCPU);
	m_particleSpawnPropertyCPU.minEmitRadians = minEmitRadians;
	m_particleSpawnPropertyCPU.maxEmitRadians = maxEmitRadians;
	m_particleSpawnPropertyCPU.emitSpeed = emitSpeed;

	m_isParticleSpawnPropertyChanged = true;
}

void CParticleSpawnProperty::SetMinEmitRadians(const XMFLOAT2& minEmitRadians)
{
	m_particleSpawnPropertyCPU.minEmitRadians = minEmitRadians;
	m_isParticleSpawnPropertyChanged = true;
}

void CParticleSpawnProperty::SetMaxEmitRadians(const XMFLOAT2& maxEmitRadians)
{
	m_particleSpawnPropertyCPU.maxEmitRadians = maxEmitRadians;
	m_isParticleSpawnPropertyChanged = true;
}


void CParticleSpawnProperty::SetEmitSpeed(float emitSpeed)
{
	m_particleSpawnPropertyCPU.emitSpeed = emitSpeed;
	m_isParticleSpawnPropertyChanged = true;
}

void CParticleSpawnProperty::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_particleSpawnPropertyGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_particleSpawnPropertyCPU));
	m_particleSpawnPropertyGPU->InitializeBuffer(device);
}

void CParticleSpawnProperty::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	UpdateEmitCount(dt);

	if (m_isParticleSpawnPropertyChanged)
	{
		m_particleSpawnPropertyGPU->Stage(deviceContext);
		m_particleSpawnPropertyGPU->Upload(deviceContext);
		m_isParticleSpawnPropertyChanged = false;
	}
}

void CParticleSpawnProperty::UpdateEmitCount(float dt)
{
	m_currentPlayTime += dt;
	if (m_isPlayLoop && m_loopTime < m_currentPlayTime)
	{
		m_currentPlayTime = 0.f;
	}

	if (m_emitRateProfiles.empty())
	{
		m_currentEmitRate = 0;
	}
	else
	{
		size_t emitProfileCounts = m_emitRateProfiles.size();
		for (size_t idx = 0; idx < emitProfileCounts - 1; ++idx)
		{
			float emitTimeFrom = m_emitRateProfiles[idx].time;
			float emitTimeTo = m_emitRateProfiles[idx + 1].time;

			if (emitTimeFrom - 1E-3f < m_currentPlayTime && m_currentPlayTime < emitTimeTo + 1E-3f)
			{
				int emitCountFrom = m_emitRateProfiles[idx].emitRate;
				int emitCountTo = m_emitRateProfiles[idx + 1].emitRate;

				m_currentEmitRate =
					static_cast<UINT>(max(0.f, emitCountFrom + (emitCountTo - emitCountFrom) * (m_currentPlayTime - emitTimeFrom) / (emitTimeTo - emitTimeFrom)));
			}
		}

		if (m_currentPlayTime < m_emitRateProfiles.front().time) m_currentEmitRate = m_emitRateProfiles.front().emitRate;
		else if (m_currentPlayTime > m_emitRateProfiles.back().time) m_currentEmitRate = m_emitRateProfiles.back().emitRate;
		else;
	}
	m_currentEmitRate = static_cast<UINT>(m_currentEmitRate * dt);
}
