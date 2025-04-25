#include "ParticleEmitterManager.h"

#include "EmitterManagerCommonData.h"
#include "InitialSpawnProperty.h"
#include "EmitterUpdateProperty.h"
#include "ARuntimeSpawnProperty.h"

#include "ComputeShader.h"
#include "GraphicsPSOObject.h"
#include "ConstantBuffer.h"

#include "ParticleEmitter.h"
#include "GPUInterpolater.h"
#include "MacroUtilities.h"

using namespace std;
using namespace DirectX;
using namespace D3D11;

ParticleEmitterManager::ParticleEmitterManager(UINT maxEmitterCount)
	: AEmitterManager("ParticleEmitterManager", maxEmitterCount)
{
	SParticleInterpInformation particleInterpInformation;
	AutoZeroMemory(particleInterpInformation);

	m_emitterInterpInformationCPU.resize(m_maxEmitterCount, particleInterpInformation);
	m_forcePropertyChangedEmitterIDs.reserve(m_maxEmitterCount);
}

ParticleEmitterManager& ParticleEmitterManager::GetParticleEmitterManager()
{
	static ParticleEmitterManager particleEmitterManager(100);
	return particleEmitterManager;
}

void ParticleEmitterManager::ReclaimEmitterID(UINT emitterID) noexcept
{
	AutoZeroMemory(m_emitterInterpInformationCPU[emitterID]);
	AddInterpolaterInformChangedEmitterID(emitterID);

	AEmitterManager::ReclaimEmitterID(emitterID);
}

void ParticleEmitterManager::AddInterpolaterInformChangedEmitterID(UINT emitterID)
{
	m_interpInformationChangedEmitterIDs.emplace_back(emitterID);
}

UINT ParticleEmitterManager::AddEmitter(
	XMVECTOR position,
	XMVECTOR angle,
	ID3D11Device* device, 
	ID3D11DeviceContext* deviceContext
)
{
	UINT particleEmitterID = IssueAvailableEmitterID();
	unique_ptr<ParticleEmitter> particleEmitter = make_unique<ParticleEmitter>(
		particleEmitterID,
		position, angle,
		[this](UINT emitterID, const XMMATRIX& worldTransform) { 
			m_worldTransformCPU[emitterID] = worldTransform; 
			AddWorldTransformChangedEmitterID(emitterID); 
		},
		[this](UINT emitterID, const SEmitterForceProperty& forceProperty) 
		{
			m_forcePropertyCPU[emitterID] = forceProperty;
			AddForceChangedEmitterID(emitterID);
		},
		[this](UINT emitterID, const SParticleInterpInformation& interpInformation) 
		{
			m_emitterInterpInformationCPU[emitterID] = interpInformation;
			AddInterpolaterInformChangedEmitterID(emitterID);
		}
	);

	particleEmitter->Initialize(device, deviceContext);

	m_emitters.emplace_back(std::move(particleEmitter));
	return particleEmitterID;
}

void ParticleEmitterManager::InitializeImpl(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	AEmitterManager::InitializeImpl(device, deviceContext);

	m_emitterInterpInformationGPU = make_unique<CStructuredBuffer>(
		static_cast<UINT>(sizeof(SParticleInterpInformation)),
		m_maxEmitterCount,
		m_emitterInterpInformationCPU.data()
	);
	m_emitterInterpInformationGPU->InitializeBuffer(device);
}

void ParticleEmitterManager::UpdateImpl(ID3D11DeviceContext* deviceContext, float dt)
{
	AEmitterManager::UpdateImpl(deviceContext, dt);

	UINT interpInformationChangedCount = static_cast<UINT>(m_interpInformationChangedEmitterIDs.size());
	if (interpInformationChangedCount > 0)
	{
		m_emitterInterpInformationGPU->StageNthElement(deviceContext, m_interpInformationChangedEmitterIDs.data(), interpInformationChangedCount);
		m_emitterInterpInformationGPU->UploadNthElement(deviceContext, m_interpInformationChangedEmitterIDs.data(), interpInformationChangedCount);
		m_interpInformationChangedEmitterIDs.clear();
	}
}

void ParticleEmitterManager::InitializeAliveFlag(ID3D11DeviceContext* deviceContext)
{
	ID3D11ShaderResourceView* selectSetSRVs[] = { 
		m_emitterInterpInformationGPU->GetSRV(),
		CGPUInterpolater<4, 2>::GInterpPropertyGPU->GetSRV(), 
		CGPUInterpolater<4, 4>::GInterpPropertyGPU->GetSRV() 
	};
	ID3D11ShaderResourceView* selectSetNullSRVs[] = { nullptr, nullptr, nullptr };
	ID3D11UnorderedAccessView* selectSetUavs[] = { 
		CEmitterManagerCommonData::GTotalParticles->GetUAV(),
		CEmitterManagerCommonData::GDeathIndexSet->GetUAV(),
		m_aliveIndexSet->GetUAV()
	};
	ID3D11UnorderedAccessView* selectSetNullUavs[] = { nullptr, nullptr, nullptr };

	UINT initDeathParticleCount[] = { NULL, NULL, NULL };

	CEmitterManagerCommonData::GInitializeParticleSetCS->SetShader(deviceContext);

	deviceContext->CSSetShaderResources(0, 3, selectSetSRVs);
	deviceContext->CSSetUnorderedAccessViews(0, 3, selectSetUavs, initDeathParticleCount);
	static const UINT dispatchX = static_cast<UINT>(ceil(CEmitterManagerCommonData::GParticleMaxCount / LocalThreadCount));
	deviceContext->Dispatch(dispatchX, 1, 1);
	deviceContext->CSSetShaderResources(0, 3, selectSetNullSRVs);
	deviceContext->CSSetUnorderedAccessViews(0, 3, selectSetNullUavs, initDeathParticleCount);
}

void ParticleEmitterManager::DrawParticles(ID3D11DeviceContext* deviceContext)
{
	ID3D11ShaderResourceView* patriclesSrvs[] = {
		CEmitterManagerCommonData::GTotalParticles->GetSRV(),
		m_aliveIndexSet->GetSRV()
	};
	ID3D11ShaderResourceView* patriclesNullSrvs[] = { nullptr, nullptr };

	const float blendColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	CEmitterManagerCommonData::GDrawParticlePSO->ApplyPSO(deviceContext, blendColor, 0xFFFFFFFF);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	deviceContext->VSSetShaderResources(0, 2, patriclesSrvs);

	deviceContext->DrawInstancedIndirect(m_drawIndirectBuffer->GetBuffer(), NULL);

	deviceContext->VSSetShaderResources(0, 2, patriclesNullSrvs);
	CEmitterManagerCommonData::GDrawParticlePSO->RemovePSO(deviceContext);
}
