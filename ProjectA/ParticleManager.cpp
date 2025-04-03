#include "ParticleManager.h"

#include "MacroUtilities.h"

#include "ModelFactory.h"
#include "ConstantBuffer.h"

#include "VertexShader.h"
#include "PixelShader.h"
#include "GraphicsPSOObject.h"

#include "RasterizerState.h"
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

CParticleManager::CParticleManager(UINT maxEmitterCount)
	: m_maxEmitterCount(maxEmitterCount), m_isEmitterWorldTransformationChanged(false)
{
	AutoZeroMemory(m_particleManagerPropteriesCPU);
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

void CParticleManager::PresetParticleSet()
{
}

void CParticleManager::ActivateEmitter()
{
}

void CParticleManager::DeframentPool()
{
}

void CParticleManager::SimulateParticles()
{
}

void CParticleManager::SortParticles()
{
}

void CParticleManager::DrawParticles()
{
}
