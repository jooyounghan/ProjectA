#include "ParticleManager.h"

#include "DefineLinkedWithShader.h"
#include "MacroUtilities.h"

#include "ModelFactory.h"
#include "ConstantBuffer.h"

#include "ComputeShader.h"
#include "VertexShader.h"
#include "GeometryShader.h"
#include "PixelShader.h"
#include "GraphicsPSOObject.h"

#include "RasterizerState.h"
#include "BlendState.h"
#include "DepthStencilState.h"

using namespace std;
using namespace DirectX;
using namespace D3D11;

#define ZERO_MATRIX  XMMATRIX(XMVectorZero(), XMVectorZero(), XMVectorZero(), XMVectorZero())

const vector<XMFLOAT3> CParticleManager::GEmitterBoxPositions = ModelFactory::CreateBoxPositions(XMVectorSet(1.f, 1.f, 1.f, 0.f));
const vector<UINT> CParticleManager::GEmitterBoxIndices = ModelFactory::CreateIndices();
unique_ptr<CVertexShader> CParticleManager::GEmitterDrawVS = make_unique<CVertexShader>(5);
unique_ptr<CPixelShader> CParticleManager::GEmitterDrawPS = make_unique<CPixelShader>();
unique_ptr<CGraphicsPSOObject> CParticleManager::GDrawEmitterPSO = nullptr;
unique_ptr<CConstantBuffer> CParticleManager::GEmitterPositionBuffer = nullptr;
unique_ptr<CConstantBuffer> CParticleManager::GEmitterIndexBuffer = nullptr;

void CParticleManager::InitializeEmitterDrawPSO(ID3D11Device* device)
{
	GEmitterPositionBuffer = make_unique<CConstantBuffer>(
		12, static_cast<UINT>(GEmitterBoxPositions.size()), GEmitterBoxPositions.data(), D3D11_BIND_VERTEX_BUFFER
	);
	GEmitterPositionBuffer->InitializeBuffer(device);

	GEmitterIndexBuffer = make_unique<CConstantBuffer>(
		4, static_cast<UINT>(GEmitterBoxIndices.size()), GEmitterBoxIndices.data(), D3D11_BIND_INDEX_BUFFER
	);
	GEmitterIndexBuffer->InitializeBuffer(device);

	GEmitterDrawVS->AddInputLayoutElement(
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	);
	GEmitterDrawVS->AddInputLayoutElement(
		{ "INSTANCE_WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	);
	GEmitterDrawVS->AddInputLayoutElement(
		{ "INSTANCE_WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	);
	GEmitterDrawVS->AddInputLayoutElement(
		{ "INSTANCE_WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	);
	GEmitterDrawVS->AddInputLayoutElement(
		{ "INSTANCE_WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	);

	GEmitterDrawVS->CreateShader(L"./EmitterDrawVS.hlsl", "main", "vs_5_0", device);
	GEmitterDrawPS->CreateShader(L"./EmitterDrawPS.hlsl", "main", "ps_5_0", device);

	GDrawEmitterPSO = make_unique<CGraphicsPSOObject>(
		GEmitterDrawVS.get(),
		nullptr,
		nullptr,
		nullptr,
		GEmitterDrawPS.get(),
		CRasterizerState::GetRSWireframeCWSS(),
		nullptr,
		CDepthStencilState::GetDSDraw(),
		nullptr,
		0
	);
}

unique_ptr<CComputeShader> CParticleManager::GInitializeParticleSetCS = make_unique<CComputeShader>();

void CParticleManager::InitializeSetInitializingPSO(ID3D11Device* device)
{
	GInitializeParticleSetCS->CreateShader(L"./InitializeParticleSetCS.hlsl", "main", "cs_5_0", device);
}

unique_ptr<CComputeShader> CParticleManager::GCalculateIndexPrefixSumCS = make_unique<CComputeShader>();
unique_ptr<CComputeShader> CParticleManager::GUpdateCurrentIndicesCS = make_unique<CComputeShader>();

void CParticleManager::InitializePoolingCS(ID3D11Device* device)
{
	GCalculateIndexPrefixSumCS->CreateShader(L"./CalculateIndexPrefixSumCS.hlsl", "main", "cs_5_0", device);
	GUpdateCurrentIndicesCS->CreateShader(L"./UpdateCurrentIndicesCS.hlsl", "main", "cs_5_0", device);
}

unique_ptr<CComputeShader> CParticleManager::GParticleInitialSourceCS = make_unique<CComputeShader>();
unique_ptr<CComputeShader> CParticleManager::GParticleRuntimeSourceCS = make_unique<CComputeShader>();

void CParticleManager::InitializeEmitterSourcingCS(ID3D11Device* device)
{
	GParticleInitialSourceCS->CreateShader(L"./ParticleInitialSourceCS.hlsl", "main", "cs_5_0", device);
	GParticleRuntimeSourceCS->CreateShader(L"./ParticleRuntimeSourceCS.hlsl", "main", "cs_5_0", device);
}

unique_ptr<CComputeShader> CParticleManager::GCaculateParticleForceCS = make_unique<CComputeShader>();

void CParticleManager::InitializeParticleSimulateCS(ID3D11Device* device)
{
	GCaculateParticleForceCS->CreateShader(L"./CalculateParticleForceCS.hlsl", "main", "cs_5_0", device);
}

unique_ptr<CComputeShader> CParticleManager::GCountIndexCS = make_unique<CComputeShader>();
unique_ptr<CComputeShader> CParticleManager::GCaluclateCountPrefixSumCS = make_unique<CComputeShader>();
unique_ptr<CComputeShader> CParticleManager::GReorderCS = make_unique<CComputeShader>();

void CParticleManager::InitializeRadixSortCS(ID3D11Device* device)
{
	GCountIndexCS->CreateShader(L"./CountIndexCS.hlsl", "main", "cs_5_0", device);
	GCaluclateCountPrefixSumCS->CreateShader(L"./CalculateCountPrefixSumCS.hlsl", "main", "cs_5_0", device);
	GReorderCS->CreateShader(L"./ReorderIndexCS.hlsl", "main", "cs_5_0", device);
}

unique_ptr<CVertexShader> CParticleManager::GParticleDrawVS = make_unique<CVertexShader>(0);
unique_ptr<CGeometryShader> CParticleManager::GParticleDrawGS = make_unique<CGeometryShader>();
unique_ptr<CPixelShader> CParticleManager::GParticleDrawPS = make_unique<CPixelShader>();
unique_ptr<CGraphicsPSOObject> CParticleManager::GDrawParticlePSO = nullptr;

void CParticleManager::InitializeParticleDrawPSO(ID3D11Device* device)
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
		CBlendState::GetBSAlphaBlendSS(),
		CDepthStencilState::GetDSDraw(),
		nullptr,
		0
	);
}

CParticleManager::CParticleManager(UINT emitterTypeCount, UINT maxEmitterCount, UINT maxParticleCount)
	: m_emitterTypeCount(emitterTypeCount),
	m_emitterMaxCount(maxEmitterCount), 
	m_particleMaxCount(maxParticleCount),
	m_isEmitterWorldPositionChanged(false),
	m_isEmitterWorldTransformationChanged(false)
{
	for (UINT idx = 0; idx < m_emitterMaxCount; ++idx)
	{
		m_transformIndexQueue.push(idx);
	}

	XMMATRIX zeroMatrix = ZERO_MATRIX;
	m_emitterWorldTransformCPU.resize(m_emitterMaxCount, zeroMatrix);
	m_emitterWolrdPosCPU.resize(m_emitterMaxCount, XMVectorZero());
}

UINT CParticleManager::AddParticleEmitter(
	UINT emitterType,
	float paritlceDensity,
	float particleRadius,
	const DirectX::XMVECTOR& position, 
	const DirectX::XMVECTOR& angle, 
	const DirectX::XMFLOAT2& minInitRadians,
	const DirectX::XMFLOAT2& maxInitRadians,
	const DirectX::XMFLOAT2& minMaxRadius,
	UINT initialParticleCount,
	ID3D11Device* device, 
	ID3D11DeviceContext* deviceContext
)
{
	if (!m_transformIndexQueue.empty())
	{
		UINT emitterID = m_transformIndexQueue.front();
		m_transformIndexQueue.pop();

		m_particleEmitters.emplace_back(make_unique<CParticleEmitter>(
			emitterID, emitterType, paritlceDensity, particleRadius,
			m_isEmitterWorldPositionChanged,
			m_isEmitterWorldTransformationChanged,
			m_emitterWolrdPosCPU[emitterID],
			m_emitterWorldTransformCPU[emitterID], 
			position, angle, minInitRadians, 
			maxInitRadians, minMaxRadius, initialParticleCount
		));
		m_particleEmitters.back()->Initialize(device, deviceContext);
		return emitterID;
	}
	else
	{
		throw std::exception("Add Particle Emitter Failed");
	}
	return NULL;
}

CParticleEmitter* CParticleManager::GetEmitter(UINT emitterID)
{
	vector<unique_ptr<CParticleEmitter>>::iterator iter;
	if (FindEmitterFromID(emitterID, iter))
	{
		return iter->get();
	}
	else
	{
		return nullptr;
	}
}

void CParticleManager::RemoveParticleEmitter(UINT emitterID)
{
	vector<unique_ptr<CParticleEmitter>>::iterator iter;
	if (FindEmitterFromID(emitterID, iter))
	{
		m_transformIndexQueue.push(emitterID);
		m_emitterWorldTransformCPU[emitterID] = ZERO_MATRIX;
		m_isEmitterWorldTransformationChanged = true;
		m_particleEmitters.erase(iter);
	}
	else 
	{
		throw exception("Remove Emitter With ID Failed");
	}
}

bool CParticleManager::FindEmitterFromID(
	UINT emitterID, 
	OUT std::vector<std::unique_ptr<CParticleEmitter>>::iterator& iter
)
{
	iter = std::find_if(
		m_particleEmitters.begin(),
		m_particleEmitters.end(),
		[emitterID](const std::unique_ptr<CParticleEmitter>& emitter)
		{
			return emitter->GetEmitterID() == emitterID;
		}
	);

	return iter != m_particleEmitters.end();
}

void CParticleManager::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_emitterWorldTransformGPU = make_unique<CDynamicBuffer>(
		static_cast<UINT>(sizeof(XMMATRIX)),
		static_cast<UINT>(m_emitterWorldTransformCPU.size()),
		m_emitterWorldTransformCPU.data(),
		D3D11_BIND_VERTEX_BUFFER
	);
	m_emitterWorldTransformGPU->InitializeBuffer(device);

	m_emitterWorldPosGPU = make_unique<CStructuredBuffer>(
		static_cast<UINT>(sizeof(XMVECTOR)),
		static_cast<UINT>(m_emitterWolrdPosCPU.size()),
		m_emitterWolrdPosCPU.data()
	);
	m_emitterWorldPosGPU->InitializeBuffer(device);


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

	vector<D3D11_DISPATCH_INDIRECT_ARGS> emitterPerDispatchIndirectArgs;
	emitterPerDispatchIndirectArgs.resize(m_emitterTypeCount, dispatchIndirectArgs);
	m_emitterPerTypeDispatchIndirectBuffer = make_unique<CIndirectBuffer< D3D11_DISPATCH_INDIRECT_ARGS>>(m_emitterTypeCount, emitterPerDispatchIndirectArgs.data());
	m_emitterPerTypeDispatchIndirectBuffer->InitializeBuffer(device);

	m_totalParticles = make_unique<CStructuredBuffer>(static_cast<UINT>(sizeof(SParticle)), m_particleMaxCount, nullptr);
	m_aliveFlags = make_unique<CStructuredBuffer>(4, m_particleMaxCount, nullptr);
	m_deathParticleSet = make_unique<CAppendBuffer>(4, m_particleMaxCount, nullptr);

	m_alivePrefixSums = make_unique<CStructuredBuffer>(4, m_particleMaxCount, nullptr);
	m_alivePrefixDescriptors = make_unique<CStructuredBuffer>(static_cast<UINT>(sizeof(SPrefixDesciptor)), UINT(ceil(m_particleMaxCount / LocalThreadCount)), nullptr);
	m_indicesBuffers = make_unique<CStructuredBuffer>(static_cast<UINT>(sizeof(SParticleSelector)), m_particleMaxCount, nullptr);

	m_countBuffers = make_unique<CStructuredBuffer>(4, static_cast<UINT>(CountArraySizePerThread) * m_particleMaxCount, nullptr);
	m_countPrefixSums = make_unique<CStructuredBuffer>(4, static_cast<UINT>(CountArraySizePerThread) * m_particleMaxCount, nullptr);
	m_countPrefixDescriptors = make_unique<CStructuredBuffer>(static_cast<UINT>(sizeof(SPrefixDesciptor)), UINT(ceil(m_particleMaxCount / LocalThreadCount)), nullptr);
	m_sortedIndicesBuffers = make_unique<CStructuredBuffer>(static_cast<UINT>(sizeof(SParticleSelector)), m_particleMaxCount, nullptr);

	m_totalParticles->InitializeBuffer(device);
	m_aliveFlags->InitializeBuffer(device);
	m_deathParticleSet->InitializeBuffer(device);

	m_alivePrefixSums->InitializeBuffer(device);
	m_alivePrefixDescriptors->InitializeBuffer(device);
	m_indicesBuffers->InitializeBuffer(device);

	m_countBuffers->InitializeBuffer(device);
	m_sortedIndicesBuffers->InitializeBuffer(device);
}

void CParticleManager::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	for (auto& particleEmitter : m_particleEmitters)
	{
		if (particleEmitter) particleEmitter->Update(deviceContext, dt);
	}

	if (m_isEmitterWorldPositionChanged)
	{
		m_emitterWorldPosGPU->Stage(deviceContext);
		m_emitterWorldPosGPU->Upload(deviceContext);
		m_isEmitterWorldPositionChanged = false;
	}

	if (m_isEmitterWorldTransformationChanged)
	{
		m_emitterWorldTransformGPU->Stage(deviceContext);
		m_emitterWorldTransformGPU->Upload(deviceContext);
		m_isEmitterWorldTransformationChanged = false;
	}
}

void CParticleManager::DrawEmittersDebugCube(ID3D11DeviceContext* deviceContext)
{
	static vector<ID3D11Buffer*> vertexBuffer = { GEmitterPositionBuffer->GetBuffer(), m_emitterWorldTransformGPU->GetBuffer() };
	static vector<ID3D11Buffer*> vertexNullBuffer = vector<ID3D11Buffer*>(vertexBuffer.size(), nullptr);
	static ID3D11Buffer* indexBuffer = GEmitterIndexBuffer->GetBuffer();
	static vector<UINT> strides = { static_cast<UINT>(sizeof(XMFLOAT3)), static_cast<UINT>(sizeof(XMMATRIX)) };
	static vector<UINT> nullStrides = vector<UINT>(strides.size(), NULL);
	static vector<UINT> offsets = { 0, 0 };
	static vector<UINT> nullOffsets = vector<UINT>(nullStrides.size(), NULL);

	GDrawEmitterPSO->ApplyPSO(deviceContext);
	
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	deviceContext->IASetVertexBuffers(0, static_cast<UINT>(vertexBuffer.size()), vertexBuffer.data(), strides.data(), offsets.data());
	deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, NULL);
	deviceContext->DrawIndexedInstanced(
		static_cast<UINT>(GEmitterBoxIndices.size()),
		m_emitterMaxCount,
		NULL, NULL, NULL
	);
	deviceContext->IASetVertexBuffers(0, static_cast<UINT>(vertexNullBuffer.size()), vertexNullBuffer.data(), nullStrides.data(), nullOffsets.data());
	deviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, NULL);

	GDrawEmitterPSO->RemovePSO(deviceContext);
}

void CParticleManager::ExecuteParticleSystem(ID3D11DeviceContext* deviceContext)
{
	InitializeParticleSet(deviceContext);
	SourceEmitter(deviceContext);
	PoolingParticles(deviceContext);

}

void CParticleManager::InitializeParticleSet(ID3D11DeviceContext* deviceContext)
{
	const static UINT dispatchX = UINT(ceil(m_particleMaxCount / LocalThreadCount));

	ID3D11UnorderedAccessView* selectSetUavs[] = { m_particleDrawIndirectStagingGPU->GetUAV(), m_totalParticles->GetUAV(), m_aliveFlags->GetUAV(), m_deathParticleSet->GetUAV(), m_alivePrefixDescriptors->GetUAV() };
	ID3D11UnorderedAccessView* selectSetNullUavs[] = { nullptr, nullptr, nullptr, nullptr, nullptr };

	UINT initDeathParticleCount[] = { NULL, NULL, NULL, 0, NULL};
	GInitializeParticleSetCS->SetShader(deviceContext);

	deviceContext->CSSetUnorderedAccessViews(0, 5, selectSetUavs, initDeathParticleCount);
	deviceContext->Dispatch(dispatchX, 1, 1);
	deviceContext->CSSetUnorderedAccessViews(0, 5, selectSetNullUavs, initDeathParticleCount);
}

void CParticleManager::SourceEmitter(ID3D11DeviceContext* deviceContext)
{
	ID3D11UnorderedAccessView* selectSetUavs[] = { m_totalParticles->GetUAV(), m_aliveFlags->GetUAV(), m_deathParticleSet->GetUAV() };
	ID3D11UnorderedAccessView* selectSetNullUavs[] = { nullptr, nullptr, nullptr };
	UINT initialValue[3] = { NULL, NULL, static_cast<UINT>(-1) };

	deviceContext->CSSetUnorderedAccessViews(0, 3, selectSetUavs, initialValue);

#pragma region 초기화 소싱
	GParticleInitialSourceCS->SetShader(deviceContext);
	for (auto& particleEmitter : m_particleEmitters)
	{
		CEmitterSpawnProperty* emitterSpawnProperty = particleEmitter->GetEmitterSpawnProperty();
		if (!emitterSpawnProperty->IsSpawned())
		{
			ID3D11Buffer* propertiesBuffer[] = { particleEmitter->GetEmitterPropertyBuffer(), emitterSpawnProperty->GetEmitterSpawnPropertyBuffer() };
			deviceContext->CSSetConstantBuffers(2, 2, propertiesBuffer);
		
			const UINT dispatchX = UINT(ceil(emitterSpawnProperty->GetInitialParticleCount() / LocalThreadCount));
			deviceContext->Dispatch(dispatchX, 1, 1);
			emitterSpawnProperty->SetSpawned();
		}
	}
#pragma endregion

#pragma region 런타임 소싱
	GParticleRuntimeSourceCS->SetShader(deviceContext);
	for (auto& particleEmitter : m_particleEmitters)
	{
		CParticleSpawnProperty* particleSpawnProperty = particleEmitter->GetParticleSpawnProperty();
		UINT emitRate = particleSpawnProperty->GetCurrentEmitRate();
		if (emitRate > 0)
		{
			ID3D11Buffer* propertiesBuffer[] = { particleEmitter->GetEmitterPropertyBuffer(), particleSpawnProperty->GetParticleSpawnPropertyBuffer() };
			deviceContext->CSSetConstantBuffers(2, 2, propertiesBuffer);
			deviceContext->Dispatch(emitRate, 1, 1);
		}
	}	
#pragma endregion
	ID3D11Buffer* emitterPropertiesNullBuffer[] = { nullptr, nullptr };
	deviceContext->CSSetConstantBuffers(2, 2, emitterPropertiesNullBuffer);

	deviceContext->CSSetUnorderedAccessViews(0, 3, selectSetNullUavs, nullptr);
}

void CParticleManager::PoolingParticles(ID3D11DeviceContext* deviceContext)
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
}

void CParticleManager::SortingParitcleIndices(ID3D11DeviceContext* deviceContext)
{

}

void CParticleManager::CaculateParticlesForce(ID3D11DeviceContext* deviceContext)
{
	ID3D11ShaderResourceView* simulateSrvs[] = { m_particleDrawIndirectStagingGPU->GetSRV(), m_indicesBuffers->GetSRV(), m_emitterWorldPosGPU->GetSRV() };
	ID3D11ShaderResourceView* simulateNullSrvs[] = { nullptr, nullptr, nullptr };

	ID3D11UnorderedAccessView* simulateUav = m_totalParticles->GetUAV();
	ID3D11UnorderedAccessView* simulateNullUav = nullptr;

	GCaculateParticleForceCS->SetShader(deviceContext);

	deviceContext->CSSetShaderResources(0, 3, simulateSrvs);
	deviceContext->CSSetUnorderedAccessViews(0, 1, &simulateUav, nullptr);

	deviceContext->CopyResource(m_indexedParticleDispatchIndirectBuffer->GetBuffer(), m_IndexedParticleDispatchIndirectStagingGPU->GetBuffer());
	deviceContext->DispatchIndirect(m_indexedParticleDispatchIndirectBuffer->GetBuffer(), NULL);

	deviceContext->CSSetShaderResources(0, 3, simulateNullSrvs);
	deviceContext->CSSetUnorderedAccessViews(0, 1, &simulateNullUav, nullptr);
}

void CParticleManager::DrawParticles(ID3D11DeviceContext* deviceContext)
{
	ID3D11ShaderResourceView* patriclesSrvs[] = { m_totalParticles->GetSRV(), m_indicesBuffers->GetSRV() };
	ID3D11ShaderResourceView* patriclesNullSrvs[] = { nullptr, nullptr };

	const float blendColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GDrawParticlePSO->ApplyPSO(deviceContext, blendColor, 0xFFFFFFFF);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	deviceContext->VSSetShaderResources(0, 2, patriclesSrvs);

	deviceContext->CopyResource(m_particleDrawIndirectBuffer->GetBuffer(), m_particleDrawIndirectStagingGPU->GetBuffer());
	deviceContext->DrawInstancedIndirect(m_particleDrawIndirectBuffer->GetBuffer(), NULL);

	deviceContext->VSSetShaderResources(0, 2, patriclesNullSrvs);
	GDrawParticlePSO->RemovePSO(deviceContext);
}
