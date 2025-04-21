#include "AEmitter.h"
#include "MacroUtilities.h"
#include "BufferMacroUtilities.h"

#include "ComputeShader.h"
#include "VertexShader.h"
#include "GeometryShader.h"
#include "PixelShader.h"
#include "GraphicsPSOObject.h"

#include "RasterizerState.h"
#include "BlendState.h"
#include "DepthStencilState.h"

#include "ModelFactory.h"

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

const vector<XMFLOAT3> AEmitter::GEmitterBoxPositions = ModelFactory::CreateBoxPositions(XMVectorSet(1.f, 1.f, 1.f, 0.f));
const vector<UINT> AEmitter::GEmitterBoxIndices = ModelFactory::CreateIndices();
unique_ptr<CVertexShader> AEmitter::GEmitterDrawVS = make_unique<CVertexShader>(5);
unique_ptr<CPixelShader> AEmitter::GEmitterDrawPS = make_unique<CPixelShader>();
unique_ptr<CGraphicsPSOObject> AEmitter::GDrawEmitterPSO = nullptr;
unique_ptr<CConstantBuffer> AEmitter::GEmitterPositionBuffer = nullptr;
unique_ptr<CConstantBuffer> AEmitter::GEmitterIndexBuffer = nullptr;

void AEmitter::InitializeGlobalEmitterProperty(UINT emitterMaxCount, ID3D11Device* device)
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
	GEmitterWorldTransformCPU.resize(GEmitterMaxCount, ZERO_MATRIX);
	GEmitterForcePropertyCPU.resize(GEmitterMaxCount, initialForceProperty);

	GEmitterWorldTransformGPU = make_unique<CDynamicBuffer>(
		static_cast<UINT>(sizeof(XMMATRIX)),
		static_cast<UINT>(GEmitterWorldTransformCPU.size()),
		GEmitterWorldTransformCPU.data(),
		D3D11_BIND_VERTEX_BUFFER
	);
	GEmitterForcePropertyGPU = make_unique<CStructuredBuffer>(
		static_cast<UINT>(sizeof(SEmitterForceProperty)),
		static_cast<UINT>(GEmitterForcePropertyCPU.size()),
		GEmitterForcePropertyCPU.data()
	);

	GEmitterWorldTransformGPU->InitializeBuffer(device);
	GEmitterForcePropertyGPU->InitializeBuffer(device);
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

void AEmitter::ReclaimEmitterID(UINT emitterID)
{
	GEmitterIDQueue.push(emitterID);
}

void AEmitter::InitializeEmitterDrawPSO(ID3D11Device* device)
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

void AEmitter::DrawEmittersDebugCube(ID3D11DeviceContext* deviceContext)
{
	static vector<ID3D11Buffer*> vertexBuffer = { GEmitterPositionBuffer->GetBuffer(), GEmitterWorldTransformGPU->GetBuffer() };
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
		GEmitterMaxCount,
		NULL, NULL, NULL
	);
	deviceContext->IASetVertexBuffers(0, static_cast<UINT>(vertexNullBuffer.size()), vertexNullBuffer.data(), nullStrides.data(), nullOffsets.data());
	deviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, NULL);

	GDrawEmitterPSO->RemovePSO(deviceContext);
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
) :
	m_isSpawned(false),
	m_position(position),
	m_angle(angle),
	m_isEmitterWorldTransformChanged(isEmitterWorldTransformChanged),
	m_emitterWorldTransform(emitterWorldTransform),
	m_isEmitterForceChanged(isEmitterForceChanged),
	m_emitterForce(emitterForce),
	m_isEmitterPropertyChanged(false),
	m_currnetEmitter(0.f),
	m_loopTime(10.f)
{
	AutoZeroMemory(m_emitterPropertyCPU);
	m_emitterPropertyCPU.emitterType = emitterType;
	m_emitterPropertyCPU.emitterID = emitterID;
	m_emitterPropertyCPU.emitterWorldTransform = emitterWorldTransform;
	m_isEmitterPropertyChanged = true;
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
	m_isEmitterPropertyChanged = true;
}

void AEmitter::SetAngle(const XMVECTOR& angle) noexcept
{
	m_angle = angle;
	m_isEmitterPropertyChanged = true;
}

#define INITIALIZE_PROPRTY(PROPERTY, D, DC) if (PROPERTY) PROPERTY->Initialize(D, DC);

void AEmitter::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_emitterPropertyGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_emitterPropertyCPU));
	m_emitterPropertyGPU->InitializeBuffer(device);

	INITIALIZE_PROPRTY(m_emitterSpawnProperty, device, deviceContext);
	INITIALIZE_PROPRTY(m_emitterUpdateProperty, device, deviceContext);
	INITIALIZE_PROPRTY(m_particleSpawnProperty, device, deviceContext);
	INITIALIZE_PROPRTY(m_particleUpdateProperty, device, deviceContext);
}

#define UPDATE_PROPRTY(PROPERTY, DC, DT) if (PROPERTY) PROPERTY->Update(DC, DT);

void AEmitter::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	if (m_isEmitterPropertyChanged)
	{
		m_emitterWorldTransform = XMMatrixAffineTransformation(
			XMVectorSet(1.f, 1.f, 1.f, 0.f),
			XMQuaternionIdentity(),
			XMQuaternionRotationRollPitchYawFromVector(m_angle),
			m_position
		);
		m_emitterPropertyCPU.emitterWorldTransform = XMMatrixTranspose(m_emitterWorldTransform);

		m_emitterPropertyGPU->Stage(deviceContext);
		m_emitterPropertyGPU->Upload(deviceContext);

		m_isEmitterPropertyChanged = false;
		m_isEmitterWorldTransformChanged = true;
	}
	UPDATE_PROPRTY(m_emitterSpawnProperty, deviceContext, dt);
	UPDATE_PROPRTY(m_emitterUpdateProperty, deviceContext, dt);
	UPDATE_PROPRTY(m_particleSpawnProperty, deviceContext, dt);
	UPDATE_PROPRTY(m_particleUpdateProperty, deviceContext, dt);
}
