#include "ParticleManager.h"

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

unique_ptr<CComputeShader> CParticleManager::GSelectParticleSetCS = make_unique<CComputeShader>();
unique_ptr<CComputeShader> CParticleManager::GDefragmenaPoolCS = make_unique<CComputeShader>();

void CParticleManager::InitializePoolingPSO(ID3D11Device* device)
{
	GSelectParticleSetCS->CreateShader(L"./SelectParticleSetCS.hlsl", "main", "cs_5_0", device);
	GDefragmenaPoolCS->CreateShader(L"./DefragmentPoolCS.hlsl", "main", "cs_5_0", device);
}

unique_ptr<CComputeShader> CParticleManager::GParticleSourcingCS = make_unique<CComputeShader>();

void CParticleManager::InitializeEmitterSourcingPSO(ID3D11Device* device)
{
	GParticleSourcingCS->CreateShader(L"./ParticleSourceCS.hlsl", "main", "cs_5_0", device);
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

CParticleManager::CParticleManager(UINT maxEmitterCount, UINT maxParticleCount)
	: m_maxEmitterCount(maxEmitterCount), 
	m_maxParticleCount(maxParticleCount),
	m_isEmitterWorldTransformationChanged(false)
{
	for (UINT idx = 0; idx < m_maxEmitterCount; ++idx)
	{
		m_transformIndexQueue.push(idx);
	}

	XMMATRIX zeroMatrix = ZERO_MATRIX;
	m_emitterWorldTransformCPU.resize(m_maxEmitterCount, zeroMatrix);
}

UINT CParticleManager::AddParticleEmitter(
	const DirectX::XMVECTOR& position, 
	const DirectX::XMVECTOR& angle, 
	const DirectX::XMVECTOR& emitVelocity,
	ID3D11Device* device, ID3D11DeviceContext* deviceContext
)
{
	if (!m_transformIndexQueue.empty())
	{
		UINT emitterID = m_transformIndexQueue.front();
		m_transformIndexQueue.pop();

		m_particleEmitters.emplace_back(make_unique<CParticleEmitter>(
			emitterID, m_isEmitterWorldTransformationChanged,
			m_emitterWorldTransformCPU[emitterID], position, angle, emitVelocity
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

void CParticleManager::RemoveParticleEmitter(UINT emitterID)
{
	auto it = std::find_if(
		m_particleEmitters.begin(),
		m_particleEmitters.end(),
		[emitterID](const std::unique_ptr<CParticleEmitter>& emitter)
		{
			return emitter->GetEmitterID() == emitterID;
		}
	);

	if (it != m_particleEmitters.end()) 
	{
		m_transformIndexQueue.push(emitterID);
		m_emitterWorldTransformCPU[emitterID] = ZERO_MATRIX;
		m_isEmitterWorldTransformationChanged = true;
		m_particleEmitters.erase(it); 
	}
	else 
	{
		throw exception("Remove Emitter With ID Failed");
	}
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

	m_particleCountsGPU = make_unique<CStructuredBuffer>(4, 4, nullptr);
	m_particleCountsGPU->InitializeBuffer(device);

	D3D11_DISPATCH_INDIRECT_ARGS dispatchIndirectArgs;
	AutoZeroMemory(dispatchIndirectArgs);
	dispatchIndirectArgs.threadGroupCountX = UINT(ceil(m_maxParticleCount / 64.f));
	dispatchIndirectArgs.threadGroupCountY = 1;
	dispatchIndirectArgs.threadGroupCountZ = 1;
	m_particleDispatchBuffer = make_unique<CIndirectBuffer<D3D11_DISPATCH_INDIRECT_ARGS>>(1, &dispatchIndirectArgs);
	m_particleDispatchBuffer->InitializeBuffer(device);

	m_totalParticlePool = make_unique<CStructuredBuffer>(static_cast<UINT>(sizeof(SParticle)), m_maxParticleCount, nullptr);
	m_deathParticleSet = make_unique<CAppendBuffer>(4, m_maxParticleCount, nullptr);
	m_aliveParticleSet = make_unique<CAppendBuffer>(4, m_maxParticleCount, nullptr);
	m_totalParticlePool->InitializeBuffer(device);
	m_deathParticleSet->InitializeBuffer(device);
	m_aliveParticleSet->InitializeBuffer(device);
}

void CParticleManager::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	for (auto& particleEmitter : m_particleEmitters)
	{
		if (particleEmitter) particleEmitter->Update(deviceContext, dt);
	}

	if (m_isEmitterWorldTransformationChanged)
	{
		m_emitterWorldTransformGPU->Stage(deviceContext);
		m_emitterWorldTransformGPU->Upload(deviceContext);
		m_isEmitterWorldTransformationChanged = false;
	}
}

void CParticleManager::DrawEmittersDebugCube(ID3D11Buffer* viewProjBuffer, ID3D11DeviceContext* deviceContext)
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
	deviceContext->VSSetConstantBuffers(0, 1, &viewProjBuffer);
	deviceContext->DrawIndexedInstanced(
		static_cast<UINT>(GEmitterBoxIndices.size()),
		m_maxEmitterCount,
		NULL, NULL, NULL
	);

	deviceContext->VSSetConstantBuffers(0, 1, &viewProjBuffer);
	deviceContext->IASetVertexBuffers(0, static_cast<UINT>(vertexNullBuffer.size()), vertexNullBuffer.data(), nullStrides.data(), nullOffsets.data());
	deviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, NULL);

	GDrawEmitterPSO->RemovePSO(deviceContext);
}

void CParticleManager::ExecuteParticleSystem(ID3D11DeviceContext* deviceContext)
{
	PresetParticleSet(deviceContext);
	ActivateEmitter(deviceContext);
}

void CParticleManager::PresetParticleSet(ID3D11DeviceContext* deviceContext)
{
	ID3D11ShaderResourceView* selectSetSrv = m_totalParticlePool->GetSRV();
	ID3D11UnorderedAccessView* selectSetUavs[] = { m_deathParticleSet->GetUAV(), m_aliveParticleSet->GetUAV() };
	ID3D11ShaderResourceView* selectSetNullSrv = nullptr;
	ID3D11UnorderedAccessView* selectSetNullUavs[] = { nullptr, nullptr };

	UINT initDeathParticleCount = 0;
	GSelectParticleSetCS->SetShader(deviceContext);

	deviceContext->CSSetShaderResources(0, 1, &selectSetSrv);
	deviceContext->CSSetUnorderedAccessViews(0, 2, selectSetUavs, &initDeathParticleCount);

	deviceContext->DispatchIndirect(m_particleDispatchBuffer->GetBuffer(), 0);

	deviceContext->CSSetShaderResources(0, 1, &selectSetNullSrv);
	deviceContext->CSSetUnorderedAccessViews(0, 2, selectSetNullUavs, &initDeathParticleCount);
}

void CParticleManager::ActivateEmitter(ID3D11DeviceContext* deviceContext)
{
	ID3D11UnorderedAccessView* selectSetUavs[] = { m_deathParticleSet->GetUAV(), m_aliveParticleSet->GetUAV(), m_totalParticlePool->GetUAV() };
	ID3D11UnorderedAccessView* selectSetNullUavs[] = { nullptr, nullptr, nullptr };
	UINT initialValue[3] = { static_cast<UINT>(-1), static_cast<UINT>(-1), NULL };
	GParticleSourcingCS->SetShader(deviceContext);

	deviceContext->CSSetUnorderedAccessViews(0, 3, selectSetUavs, initialValue);
	for (auto& particleEmitter : m_particleEmitters)
	{
		ID3D11Buffer* emitterPropertiesBuffer = particleEmitter->GetEmitterPropertiesBuffer();
		deviceContext->CSSetConstantBuffers(1, 1, &emitterPropertiesBuffer);
		deviceContext->Dispatch(1, 1, 1);
	}	
	ID3D11Buffer* emitterPropertiesNullBuffer = nullptr;
	deviceContext->CSSetConstantBuffers(1, 1, &emitterPropertiesNullBuffer);
	deviceContext->CSSetUnorderedAccessViews(0, 3, selectSetNullUavs, nullptr);
}

void CParticleManager::DeframentPool(ID3D11DeviceContext* deviceContext)
{
}

void CParticleManager::SimulateParticles(ID3D11DeviceContext* deviceContext)
{
}

void CParticleManager::SortParticles(ID3D11DeviceContext* deviceContext)
{
}

void CParticleManager::DrawParticles(ID3D11DeviceContext* deviceContext)
{
	ID3D11ShaderResourceView* tempPatriclesSrv = m_totalParticlePool->GetSRV();
	ID3D11ShaderResourceView* tempPatriclesNullSrv = nullptr;

	const float blendColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	GDrawParticlePSO->ApplyPSO(deviceContext, blendColor, 0xFFFFFFFF);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	deviceContext->VSSetShaderResources(0, 1, &tempPatriclesSrv);

	deviceContext->DrawInstanced(m_maxParticleCount, 1, NULL, NULL);

	deviceContext->VSSetShaderResources(0, 1, &tempPatriclesNullSrv);

	GDrawParticlePSO->RemovePSO(deviceContext);
}
