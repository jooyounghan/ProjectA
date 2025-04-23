#include "EmitterManager.h"

#include "DefineLinkedWithShader.h"
#include "BufferMacroUtilities.h"
#include "MacroUtilities.h"

#include "ComputeShader.h"
#include "VertexShader.h"
#include "GeometryShader.h"
#include "PixelShader.h"
#include "GraphicsPSOObject.h"

#include "StructuredBuffer.h"

#include "RasterizerState.h"
#include "BlendState.h"
#include "DepthStencilState.h"

#include "AEmitter.h"
#include "BaseEmitterSpawnProperty.h"
#include "BaseEmitterUpdateProperty.h"
#include "BaseParticleSpawnProperty.h"
#include "BaseParticleUpdateProperty.h"

#include "GPUInterpolater.h"

using namespace std;
using namespace DirectX;
using namespace D3D11;



unique_ptr<CComputeShader> CEmitterManager::GInitializeParticleSetCS = make_unique<CComputeShader>();

void CEmitterManager::InitializeSetInitializingPSO(ID3D11Device* device)
{
	GInitializeParticleSetCS->CreateShader(L"./InitializeParticleSetCS.hlsl", "main", "cs_5_0", device);
}

unique_ptr<CComputeShader> CEmitterManager::GCalculateIndexPrefixSumCS = make_unique<CComputeShader>();
unique_ptr<CComputeShader> CEmitterManager::GUpdateCurrentIndicesCS = make_unique<CComputeShader>();

void CEmitterManager::InitializePoolingCS(ID3D11Device* device)
{
	GCalculateIndexPrefixSumCS->CreateShader(L"./CalculateIndexPrefixSumCS.hlsl", "main", "cs_5_0", device);
	GUpdateCurrentIndicesCS->CreateShader(L"./UpdateCurrentIndicesCS.hlsl", "main", "cs_5_0", device);
}

unique_ptr<CComputeShader> CEmitterManager::GParticleInitialSourceCS = make_unique<CComputeShader>();
unique_ptr<CComputeShader> CEmitterManager::GParticleRuntimeSourceCS = make_unique<CComputeShader>();

void CEmitterManager::InitializeEmitterSourcingCS(ID3D11Device* device)
{
	GParticleInitialSourceCS->CreateShader(L"./ParticleInitialSourceCS.hlsl", "main", "cs_5_0", device);
	GParticleRuntimeSourceCS->CreateShader(L"./ParticleRuntimeSourceCS.hlsl", "main", "cs_5_0", device);
}

unique_ptr<CComputeShader> CEmitterManager::GCaculateParticleForceCS = make_unique<CComputeShader>();

void CEmitterManager::InitializeParticleSimulateCS(ID3D11Device* device)
{
	GCaculateParticleForceCS->CreateShader(L"./CalculateParticleForceCS.hlsl", "main", "cs_5_0", device);
}

unique_ptr<CComputeShader> CEmitterManager::GIndexRadixSortCS = make_unique<CComputeShader>();

void CEmitterManager::InitializeRadixSortCS(ID3D11Device* device)
{
	GIndexRadixSortCS->CreateShader(L"./IndexRadixSortCS.hlsl", "main", "cs_5_0", device);
}

unique_ptr<CVertexShader> CEmitterManager::GParticleDrawVS = make_unique<CVertexShader>(0);
unique_ptr<CGeometryShader> CEmitterManager::GParticleDrawGS = make_unique<CGeometryShader>();
unique_ptr<CPixelShader> CEmitterManager::GParticleDrawPS = make_unique<CPixelShader>();
unique_ptr<CGraphicsPSOObject> CEmitterManager::GDrawParticlePSO = nullptr;

void CEmitterManager::InitializeParticleDrawPSO(ID3D11Device* device)
{
	GParticleDrawVS->CreateShader(L"./ParticleDrawVS.hlsl", "main", "vs_5_0", device);
	GParticleDrawGS->CreateShader(L"./ParticleDrawGS.hlsl", "main", "gs_5_0", device);
	GParticleDrawPS->CreateShader(L"./ParticleDrawPS.hlsl", "main", "ps_5_0", device);

	GDrawParticlePSO = make_unique<CGraphicsPSOObject>(
		GParticleDrawVS.get(),
		nullptr,
		nullptr,
		GParticleDrawGS.get(),
		GParticleDrawPS.get(),
		CRasterizerState::GetRSSolidCWSS(),
		CBlendState::GetBSAccumulateWithAlpha(),
		CDepthStencilState::GetDSDraw(),
		nullptr,
		0
	);
}

CEmitterManager::CEmitterManager(UINT emitterTypeCount, UINT particleMaxCount)
	: /*m_emitterTypeCount(emitterTypeCount),*/
	m_particleMaxCount(particleMaxCount)
{

}

void CEmitterManager::AddParticleEmitter(
	unique_ptr<AEmitter>& emitter, 
	ID3D11Device* device, 
	ID3D11DeviceContext* deviceContext
) 
{
	emitter->Initialize(device, deviceContext);
	m_emitters.emplace_back(move(emitter)); 
}

AEmitter* CEmitterManager::GetEmitter(UINT emitterID)
{
	vector<unique_ptr<AEmitter>>::iterator iter;
	if (FindEmitterFromID(emitterID, iter))
	{
		return iter->get();
	}
	else
	{
		return nullptr;
	}
}

void CEmitterManager::RemoveParticleEmitter(UINT emitterID)
{
	vector<unique_ptr<AEmitter>>::iterator iter;
	if (FindEmitterFromID(emitterID, iter))
	{
		AEmitter::ReclaimEmitterID(emitterID);
		AEmitter::GEmitterWorldTransformCPU[emitterID] = ZERO_MATRIX;
		AEmitter::GChangedEmitterWorldPositionIDs.emplace_back(emitterID);
		m_emitters.erase(iter);
	}
	else 
	{
		throw exception("Remove Emitter With ID Failed");
	}
}

bool CEmitterManager::FindEmitterFromID(
	UINT emitterID, 
	OUT std::vector<std::unique_ptr<AEmitter>>::iterator& iter
)
{
	iter = std::find_if(
		m_emitters.begin(),
		m_emitters.end(),
		[emitterID](const std::unique_ptr<AEmitter>& emitter)
		{
			return emitter->GetEmitterID() == emitterID;
		}
	);

	return iter != m_emitters.end();
}

void CEmitterManager::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	D3D11_DRAW_INSTANCED_INDIRECT_ARGS drawIndirectArgs;
	AutoZeroMemory(drawIndirectArgs);
	drawIndirectArgs.VertexCountPerInstance = 0;
	drawIndirectArgs.InstanceCount = 1;
	drawIndirectArgs.StartVertexLocation = NULL;
	drawIndirectArgs.StartInstanceLocation = NULL;
	m_particleDrawIndirectStagingGPU = make_unique<CStructuredBuffer>(4, 4, &drawIndirectArgs);
	m_particleDrawIndirectStagingGPU->InitializeBuffer(device);
	m_particleDrawIndirectBuffer = make_unique<CIndirectBuffer<D3D11_DRAW_INSTANCED_INDIRECT_ARGS>>(1, &drawIndirectArgs);
	m_particleDrawIndirectBuffer->InitializeBuffer(device);

	D3D11_DISPATCH_INDIRECT_ARGS dispatchIndirectArgs;
	AutoZeroMemory(dispatchIndirectArgs);
	dispatchIndirectArgs.threadGroupCountX = 1;
	dispatchIndirectArgs.threadGroupCountY = 1;
	dispatchIndirectArgs.threadGroupCountZ = 1;
	m_IndexedParticleDispatchIndirectStagingGPU = make_unique<CStructuredBuffer>(4, 4, &dispatchIndirectArgs);
	m_IndexedParticleDispatchIndirectStagingGPU->InitializeBuffer(device);
	m_indexedParticleDispatchIndirectBuffer = make_unique<CIndirectBuffer<D3D11_DISPATCH_INDIRECT_ARGS>>(1, &dispatchIndirectArgs);
	m_indexedParticleDispatchIndirectBuffer->InitializeBuffer(device);

	//vector<D3D11_DISPATCH_INDIRECT_ARGS> emitterPerDispatchIndirectArgs;
	//emitterPerDispatchIndirectArgs.resize(m_emitterTypeCount, dispatchIndirectArgs);
	//m_emitterPerTypeDispatchIndirectBuffer = make_unique<CIndirectBuffer< D3D11_DISPATCH_INDIRECT_ARGS>>(m_emitterTypeCount, emitterPerDispatchIndirectArgs.data());
	//m_emitterPerTypeDispatchIndirectBuffer->InitializeBuffer(device);

	m_totalParticles = make_unique<CStructuredBuffer>(static_cast<UINT>(sizeof(SParticle)), m_particleMaxCount, nullptr);
	m_aliveFlags = make_unique<CStructuredBuffer>(4, m_particleMaxCount, nullptr);
	m_deathParticleSet = make_unique<CAppendBuffer>(4, m_particleMaxCount, nullptr);

	m_alivePrefixSums = make_unique<CStructuredBuffer>(4, m_particleMaxCount, nullptr);
	m_alivePrefixDescriptors = make_unique<CStructuredBuffer>(static_cast<UINT>(sizeof(SPrefixDesciptor)), UINT(ceil(m_particleMaxCount / LocalThreadCount)), nullptr);
	m_indicesBuffers = make_unique<CStructuredBuffer>(4, m_particleMaxCount, nullptr);

	//m_sortStatusGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_sortStatusCPU));
	//m_countBuffers = make_unique<CStructuredBuffer>(4, static_cast<UINT>(CountArraySizePerThread) * m_particleMaxCount, nullptr);
	//m_countPrefixDescriptors = make_unique<CStructuredBuffer>(static_cast<UINT>(sizeof(SPrefixDesciptor)), UINT(ceil(m_particleMaxCount / LocalThreadCount)), nullptr);
	//m_sortedIndicesBuffers = make_unique<CStructuredBuffer>(static_cast<UINT>(sizeof(SParticleSelector)), m_particleMaxCount, nullptr);

	m_totalParticles->InitializeBuffer(device);
	m_aliveFlags->InitializeBuffer(device);
	m_deathParticleSet->InitializeBuffer(device);

	m_alivePrefixSums->InitializeBuffer(device);
	m_alivePrefixDescriptors->InitializeBuffer(device);
	m_indicesBuffers->InitializeBuffer(device);

	//m_sortStatusGPU->InitializeBuffer(device);
	//m_countBuffers->InitializeBuffer(device);
	//m_countPrefixDescriptors->InitializeBuffer(device);
	//m_sortedIndicesBuffers->InitializeBuffer(device);
}

void CEmitterManager::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	for (auto& particleEmitter : m_emitters)
	{
		if (particleEmitter) particleEmitter->Update(deviceContext, dt);
	}
}

void CEmitterManager::ExecuteParticleSystem(ID3D11DeviceContext* deviceContext)
{
	InitializeParticleSet(deviceContext);
	SourceEmitter(deviceContext);
	PoolingParticles(deviceContext);
	SortingParitcleIndices(deviceContext);
}

void CEmitterManager::InitializeParticleSet(ID3D11DeviceContext* deviceContext)
{
	const static UINT dispatchX = UINT(ceil(m_particleMaxCount / LocalThreadCount));

	ID3D11ShaderResourceView* selectSetSRVs[] = { GPUInterpolater<4, 2>::GInterpolaterPropertyGPU->GetSRV(), GPUInterpolater<4, 4>::GInterpolaterPropertyGPU->GetSRV() };
	ID3D11ShaderResourceView* selectSetNullSRVs[] = { nullptr, nullptr };
	ID3D11UnorderedAccessView* selectSetUavs[] = { 
		m_particleDrawIndirectStagingGPU->GetUAV(),
		m_totalParticles->GetUAV(), 
		m_aliveFlags->GetUAV(),
		m_deathParticleSet->GetUAV(),
		m_alivePrefixDescriptors->GetUAV() 
	};
	ID3D11UnorderedAccessView* selectSetNullUavs[] = { nullptr, nullptr, nullptr, nullptr, nullptr };

	UINT initDeathParticleCount[] = { NULL, NULL, NULL, 0, NULL};
	GInitializeParticleSetCS->SetShader(deviceContext);

	deviceContext->CSSetShaderResources(0, 2, selectSetSRVs);
	deviceContext->CSSetUnorderedAccessViews(0, 5, selectSetUavs, initDeathParticleCount);
	deviceContext->Dispatch(dispatchX, 1, 1);
	deviceContext->CSSetShaderResources(0, 2, selectSetNullSRVs);
	deviceContext->CSSetUnorderedAccessViews(0, 5, selectSetNullUavs, initDeathParticleCount);
}

void CEmitterManager::SourceEmitter(ID3D11DeviceContext* deviceContext)
{
	ID3D11UnorderedAccessView* selectSetUavs[] = { m_totalParticles->GetUAV(), m_aliveFlags->GetUAV(), m_deathParticleSet->GetUAV() };
	ID3D11UnorderedAccessView* selectSetNullUavs[] = { nullptr, nullptr, nullptr };
	UINT initialValue[3] = { NULL, NULL, static_cast<UINT>(-1) };

	deviceContext->CSSetUnorderedAccessViews(0, 3, selectSetUavs, initialValue);
#pragma region 초기화 소싱
	GParticleInitialSourceCS->SetShader(deviceContext);
	for (auto& emitter : m_emitters)
	{
		if (!emitter->IsSpawned())
		{
			BaseEmitterSpawnProperty* emitterSpawnProperty = emitter->GetAEmitterSpawnProperty();
			ID3D11Buffer* emitterInitialSourceBuffers[] = { emitter->GetEmitterPropertyBuffer(), emitterSpawnProperty->GetEmitterSpawnPropertyBuffer() };
			ID3D11Buffer* emitterInitialSourceNullBuffers[] = { nullptr, nullptr };
			const UINT dispatchX = UINT(ceil(emitterSpawnProperty->GetInitialParticleCount() / LocalThreadCount));
			deviceContext->CSSetConstantBuffers(2, 2, emitterInitialSourceBuffers);
			deviceContext->Dispatch(dispatchX, 1, 1);
			deviceContext->CSSetConstantBuffers(2, 2, emitterInitialSourceNullBuffers);
			emitter->SetSpawned(true);
		}
	}
	GParticleInitialSourceCS->ResetShader(deviceContext);
#pragma endregion

#pragma region 런타임 소싱
	GParticleRuntimeSourceCS->SetShader(deviceContext);
	for (auto& emitter : m_emitters)
	{
		BaseEmitterUpdateProperty* emitterUpdateProperty = emitter->GetAEmitterUpdateProperty();
		BaseParticleSpawnProperty* particleSpawnProperty = emitter->GetAParticleSpawnProperty();

		ID3D11Buffer* emitterRuntimeSourceBuffers[] = { emitter->GetEmitterPropertyBuffer(), particleSpawnProperty->GetParticleSpawnPropertyBuffer() };
		ID3D11Buffer* emitterRuntimeSourceNullBuffers[] = { nullptr, nullptr };
		const UINT dispatchX = emitterUpdateProperty->GetSpawnCount();
		deviceContext->CSSetConstantBuffers(2, 2, emitterRuntimeSourceBuffers);
		deviceContext->Dispatch(dispatchX, 1, 1);
		deviceContext->CSSetConstantBuffers(2, 2, emitterRuntimeSourceNullBuffers);
	}	
#pragma endregion
	ID3D11Buffer* emitterPropertiesNullBuffer[] = { nullptr, nullptr };
	deviceContext->CSSetConstantBuffers(2, 2, emitterPropertiesNullBuffer);

	deviceContext->CSSetUnorderedAccessViews(0, 3, selectSetNullUavs, nullptr);
}

void CEmitterManager::PoolingParticles(ID3D11DeviceContext* deviceContext)
{
	const static UINT dispatchX = UINT(ceil(m_particleMaxCount / LocalThreadCount));
	ID3D11UnorderedAccessView* indirectArgsUavs[] = { m_particleDrawIndirectStagingGPU->GetUAV(), m_IndexedParticleDispatchIndirectStagingGPU->GetUAV() };
	ID3D11UnorderedAccessView* indirectArgsNullUavs[] = { nullptr, nullptr };
	
	deviceContext->CSSetUnorderedAccessViews(0, 2, indirectArgsUavs, nullptr);

#pragma region Prefix Sum 계산
	ID3D11ShaderResourceView* aliveFlagsSrv[] = { m_aliveFlags->GetSRV() };
	ID3D11UnorderedAccessView* prefixSumUavs[] = { m_alivePrefixSums->GetUAV(), m_alivePrefixDescriptors->GetUAV() };
	ID3D11ShaderResourceView* aliveFlagsNullSrv[] = { nullptr };
	ID3D11UnorderedAccessView* prefixSumNullUavs[] = { nullptr, nullptr };

	UINT initialValue = static_cast<UINT>(-1);

	GCalculateIndexPrefixSumCS->SetShader(deviceContext);

	deviceContext->CSSetShaderResources(0, 1, aliveFlagsSrv);
	deviceContext->CSSetUnorderedAccessViews(2, 2, prefixSumUavs, &initialValue);

	deviceContext->Dispatch(dispatchX, 1, 1);

	deviceContext->CSSetShaderResources(0, 1, aliveFlagsNullSrv);
	deviceContext->CSSetUnorderedAccessViews(2, 2, prefixSumNullUavs, nullptr);
#pragma endregion

#pragma region Index Buffer 계산
	ID3D11ShaderResourceView* currentIndicesSrvs[] = { m_aliveFlags->GetSRV(), m_alivePrefixSums->GetSRV() };
	ID3D11UnorderedAccessView* currentIndicesUav[] = { m_totalParticles->GetUAV(), m_indicesBuffers->GetUAV() };
	ID3D11ShaderResourceView* currentIndicesNullSrvs[] = { nullptr, nullptr };
	ID3D11UnorderedAccessView* currentIndicesNullUav[] = { nullptr, nullptr };

	GUpdateCurrentIndicesCS->SetShader(deviceContext);

	deviceContext->CSSetShaderResources(0, 2, currentIndicesSrvs);
	deviceContext->CSSetUnorderedAccessViews(2, 2, currentIndicesUav, nullptr);

	deviceContext->Dispatch(dispatchX, 1, 1);

	deviceContext->CSSetShaderResources(0, 2, currentIndicesNullSrvs);
	deviceContext->CSSetUnorderedAccessViews(2, 2, currentIndicesNullUav, nullptr);
#pragma endregion

	deviceContext->CSSetUnorderedAccessViews(0, 2, indirectArgsNullUavs, nullptr);

	deviceContext->CopyResource(m_particleDrawIndirectBuffer->GetBuffer(), m_particleDrawIndirectStagingGPU->GetBuffer());
	deviceContext->CopyResource(m_indexedParticleDispatchIndirectBuffer->GetBuffer(), m_IndexedParticleDispatchIndirectStagingGPU->GetBuffer());
}

void CEmitterManager::SortingParitcleIndices(ID3D11DeviceContext* deviceContext)
{
	//const UINT sortBytesCount = 64;
	//const UINT bitOffset = static_cast<UINT>(log2(CountArraySizePerThread));
	//const UINT SortPassCount = (sortBytesCount / bitOffset);

	//ID3D11Buffer* sortCbs[] = { m_sortStatusGPU->GetBuffer() };
	//ID3D11ShaderResourceView* sortSrvs[] = { m_particleDrawIndirectStagingGPU->GetSRV() };
	//ID3D11UnorderedAccessView* sortUavs[] = { m_countBuffers->GetUAV(), m_countPrefixDescriptors->GetUAV() };
	//ID3D11Buffer* sortNullCbs[] = { nullptr };
	//ID3D11ShaderResourceView* sortNullSrvs[] = { nullptr };
	//ID3D11UnorderedAccessView* sortNullUavs[] = { nullptr, nullptr };

	//deviceContext->CSSetConstantBuffers(2, 1, sortCbs);
	//deviceContext->CSSetShaderResources(0, 1, sortSrvs);
	//deviceContext->CSSetUnorderedAccessViews(0, 2, sortUavs, nullptr);
	//GIndexRadixSortCS->SetShader(deviceContext);

	//for (UINT idx = 0; idx < SortPassCount; ++idx)
	//{
	//	m_sortStatusCPU.passCount = idx;
	//	m_sortStatusCPU.bitOffset = bitOffset;
	//	m_sortStatusGPU->Stage(deviceContext);
	//	m_sortStatusGPU->Upload(deviceContext);

	//	ID3D11ShaderResourceView* passSortSrvs[] = { m_indicesBuffers->GetSRV() };
	//	ID3D11UnorderedAccessView* passSortUavs[] = { m_sortedIndicesBuffers->GetUAV() };
	//	ID3D11ShaderResourceView* passSortNullSrvs[] = { nullptr };
	//	ID3D11UnorderedAccessView* passSortNullUavs[] = { nullptr };
	//	deviceContext->CSSetShaderResources(1, 1, passSortSrvs);
	//	deviceContext->CSSetUnorderedAccessViews(2, 1, passSortUavs, nullptr);
	//	deviceContext->DispatchIndirect(m_indexedParticleDispatchIndirectBuffer->GetBuffer(), NULL);
	//	deviceContext->CSSetShaderResources(1, 1, passSortNullSrvs);
	//	deviceContext->CSSetUnorderedAccessViews(2, 1, passSortNullUavs, nullptr);

	//	CStructuredBuffer::Swap(m_indicesBuffers.get(), m_sortedIndicesBuffers.get());
	//}
	//CStructuredBuffer::Swap(m_indicesBuffers.get(), m_sortedIndicesBuffers.get());
	//deviceContext->CSSetConstantBuffers(2, 1, sortNullCbs);
	//deviceContext->CSSetShaderResources(0, 1, sortNullSrvs);
	//deviceContext->CSSetUnorderedAccessViews(0, 2, sortNullUavs, nullptr);
}

void CEmitterManager::CaculateParticlesForce(ID3D11DeviceContext* deviceContext)
{
	ID3D11ShaderResourceView* simulateSrvs[] = { m_particleDrawIndirectStagingGPU->GetSRV(), m_indicesBuffers->GetSRV(), AEmitter::GEmitterForcePropertyGPU->GetSRV() };
	ID3D11ShaderResourceView* simulateNullSrvs[] = { nullptr, nullptr, nullptr };

	ID3D11UnorderedAccessView* simulateUav = m_totalParticles->GetUAV();
	ID3D11UnorderedAccessView* simulateNullUav = nullptr;

	GCaculateParticleForceCS->SetShader(deviceContext);

	deviceContext->CSSetShaderResources(0, 3, simulateSrvs);
	deviceContext->CSSetUnorderedAccessViews(0, 1, &simulateUav, nullptr);

	deviceContext->DispatchIndirect(m_indexedParticleDispatchIndirectBuffer->GetBuffer(), NULL);

	deviceContext->CSSetShaderResources(0, 3, simulateNullSrvs);
	deviceContext->CSSetUnorderedAccessViews(0, 1, &simulateNullUav, nullptr);
}

void CEmitterManager::DrawParticles(ID3D11DeviceContext* deviceContext)
{
	ID3D11ShaderResourceView* patriclesSrvs[] = { m_totalParticles->GetSRV(), m_indicesBuffers->GetSRV() };
	ID3D11ShaderResourceView* patriclesNullSrvs[] = { nullptr, nullptr };

	const float blendColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GDrawParticlePSO->ApplyPSO(deviceContext, blendColor, 0xFFFFFFFF);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	deviceContext->VSSetShaderResources(0, 2, patriclesSrvs);

	deviceContext->DrawInstancedIndirect(m_particleDrawIndirectBuffer->GetBuffer(), NULL);

	deviceContext->VSSetShaderResources(0, 2, patriclesNullSrvs);
	GDrawParticlePSO->RemovePSO(deviceContext);
}
