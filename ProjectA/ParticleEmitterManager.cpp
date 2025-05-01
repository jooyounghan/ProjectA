#include "ParticleEmitterManager.h"

#include "EmitterManagerCommonData.h"
#include "InitialSpawnProperty.h"
#include "EmitterUpdateProperty.h"
#include "ARuntimeSpawnProperty.h"

#include "ComputeShader.h"
#include "GraphicsPSOObject.h"
#include "ConstantBuffer.h"

#include "ParticleEmitter.h"
#include "GPUInterpPropertyManager.h"
#include "MacroUtilities.h"

using namespace std;
using namespace DirectX;
using namespace D3D11;

ParticleEmitterManager::ParticleEmitterManager(
	UINT maxEmitterCount,
	UINT maxParticleCount
)
	: AEmitterManager("ParticleEmitterManager", maxEmitterCount, maxParticleCount)
{
	SParticleInterpInformation particleInterpInformation;
	ZeroMem(particleInterpInformation);

	m_emitterInterpInformationCPU.resize(m_maxEmitterCount, particleInterpInformation);
	m_forcePropertyChangedEmitterIDs.reserve(m_maxEmitterCount);
}

ParticleEmitterManager& ParticleEmitterManager::GetParticleEmitterManager()
{
	static ParticleEmitterManager particleEmitterManager(MaxParticleEmitterCount, MaxParticleCount);
	return particleEmitterManager;
}

void ParticleEmitterManager::ReclaimEmitterID(UINT emitterID) noexcept
{
	ZeroMem(m_emitterInterpInformationCPU[emitterID]);
	AddInterpolaterInformChangedEmitterID(emitterID);

	AEmitterManager::ReclaimEmitterID(emitterID);
}

void ParticleEmitterManager::CreateAliveIndexSet(ID3D11Device* device)
{
	const UINT particleMaxCount = m_emitterManagerPropertyCPU.particleMaxCount;
	m_aliveIndexSet = make_unique<CAppendBuffer>(4, particleMaxCount, nullptr);
	m_aliveIndexSet->InitializeBuffer(device);
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
		[this](UINT emitterID, UINT colorInterpolaterID, bool isColorGPUInterpolaterOn, EInterpolationMethod colorInterpolationMethod, IInterpolater<4>* colorInterpolater)
		{
			SelectColorGPUInterpolater(emitterID, colorInterpolaterID, isColorGPUInterpolaterOn, colorInterpolationMethod, colorInterpolater);
		},
		[this](UINT emitterID, UINT colorInterpolaterID, bool isColorGPUInterpolaterOn, float maxLife, EInterpolationMethod colorInterpolationMethod, IInterpolater<4>* colorInterpolater)
		{
			UpdateColorGPUInterpolater(emitterID, colorInterpolaterID, isColorGPUInterpolaterOn, maxLife, colorInterpolationMethod, colorInterpolater);
		}
	);

	particleEmitter->CreateProperty();
	m_emitters.emplace_back(std::move(particleEmitter));
	return particleEmitterID;
}

void ParticleEmitterManager::UpdateColorGPUInterpolaterImpl(
	UINT emitterID,
	UINT colorInterpolaterID, 
	bool isColorGPUInterpolaterOn, 
	float maxLife, 
	EInterpolationMethod colorInterpolationMethod, 
	IInterpolater<4>* colorInterpolater
)
{
	m_emitterInterpInformationCPU[emitterID].maxLife = maxLife;
	m_emitterInterpInformationCPU[emitterID].colorInterpolaterID = colorInterpolaterID;
	m_emitterInterpInformationCPU[emitterID].colorInterpolaterDegree = colorInterpolater->GetDegree();
	AddInterpolaterInformChangedEmitterID(emitterID);
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

void ParticleEmitterManager::InitializeAliveFlag(ID3D11DeviceContext* deviceContext)
{
	ID3D11Buffer* initializeCBs[] = { m_emitterManagerPropertyGPU->GetBuffer() };
	ID3D11Buffer* initializeNullCBs[] = { nullptr };

	ID3D11ShaderResourceView* initializeSRVs[] = { 
		m_emitterInterpInformationGPU->GetSRV(),
		m_colorD1Dim4PorpertyManager->GetGPUInterpPropertySRV(),
		m_colorD3Dim4PorpertyManager->GetGPUInterpPropertySRV()
	};
	ID3D11ShaderResourceView* initializeNullSRVs[] = { nullptr, nullptr, nullptr };
	ID3D11UnorderedAccessView* initializeUavs[] = { 
		m_totalParticles->GetUAV(),
		m_deathIndexSet->GetUAV(),
		m_aliveIndexSet->GetUAV()
	};
	ID3D11UnorderedAccessView* initializeNullUavs[] = { nullptr, nullptr, nullptr };

	UINT initDeathParticleCount[] = { NULL, NULL, NULL };

	UINT emitterTypeIndex = GetEmitterType();
	CEmitterManagerCommonData::GInitializeParticleSetCS[emitterTypeIndex]->SetShader(deviceContext);

	deviceContext->CSSetConstantBuffers(2, 1, initializeCBs);
	deviceContext->CSSetShaderResources(0, 3, initializeSRVs);
	deviceContext->CSSetUnorderedAccessViews(0, 3, initializeUavs, initDeathParticleCount);
	static const UINT dispatchX = static_cast<UINT>(ceil(m_emitterManagerPropertyCPU.particleMaxCount / LocalThreadCount));
	deviceContext->Dispatch(dispatchX, 1, 1);
	deviceContext->CSSetConstantBuffers(2, 1, initializeNullCBs);
	deviceContext->CSSetShaderResources(0, 3, initializeNullSRVs);
	deviceContext->CSSetUnorderedAccessViews(0, 3, initializeNullUavs, initDeathParticleCount);
}

void ParticleEmitterManager::FinalizeParticles(ID3D11DeviceContext* deviceContext)
{
}

void ParticleEmitterManager::DrawParticles(ID3D11DeviceContext* deviceContext)
{
	ID3D11ShaderResourceView* patriclesSrvs[] = {
		m_totalParticles->GetSRV(),
		m_aliveIndexSet->GetSRV()
	};
	ID3D11ShaderResourceView* patriclesNullSrvs[] = { nullptr, nullptr };

	UINT emitterTypeIndex = GetEmitterType();

	const float blendColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	CEmitterManagerCommonData::GDrawParticlePSO[emitterTypeIndex]->ApplyPSO(deviceContext, blendColor, 0xFFFFFFFF);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	deviceContext->VSSetShaderResources(0, 2, patriclesSrvs);

	deviceContext->DrawInstancedIndirect(m_drawIndirectBuffer->GetBuffer(), NULL);

	deviceContext->VSSetShaderResources(0, 2, patriclesNullSrvs);
	CEmitterManagerCommonData::GDrawParticlePSO[emitterTypeIndex]->RemovePSO(deviceContext);
}
