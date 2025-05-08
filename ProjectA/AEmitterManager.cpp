#include "AEmitterManager.h"
#include "EmitterManagerCommonData.h"

#include "DefineLinkedWithShader.h"
#include "MacroUtilities.h"

#include "ComputeShader.h"
#include "VertexShader.h"
#include "GeometryShader.h"
#include "PixelShader.h"
#include "GraphicsPSOObject.h"

#include "ConstantBuffer.h"
#include "StructuredBuffer.h"

#include "AEmitter.h"
#include "EmitterSelector.h"
#include "InitialSpawnProperty.h"
#include "EmitterUpdateProperty.h"
#include "ARuntimeSpawnProperty.h"
#include "ForceUpdateProperty.h"

#include "GPUInterpPropertyManager.h"

#include <format>

using namespace std;
using namespace DirectX;
using namespace D3D11;

AEmitterManager::AEmitterManager(
	const string& managerName,
	UINT maxEmitterCount,
	UINT maxParticleCount
)
	: m_managerName(managerName), m_maxEmitterCount(maxEmitterCount),
	m_emitterManagerPropertyCPU{ maxParticleCount, 0, 0, 0 },
	m_isEmitterManagerPropertyChanged(false)
{
	for (UINT idx = 0; idx < m_maxEmitterCount; ++idx)
	{
		m_emitterIDQueue.push(idx);
	}

	XMMATRIX zeroMatrix = XMMATRIX(XMVectorZero(), XMVectorZero(), XMVectorZero(), XMVectorZero());
	SEmitterForceProperty initialForceProperty;
	ZeroMem(initialForceProperty);

	m_emitters.reserve(m_maxEmitterCount);

	m_worldTransformCPU.resize(m_maxEmitterCount, zeroMatrix);
	m_worldTransformChangedEmitterIDs.reserve(m_maxEmitterCount);

	m_forcePropertyCPU.resize(m_maxEmitterCount, initialForceProperty);
	m_forcePropertyChangedEmitterIDs.reserve(m_maxEmitterCount);
}

AEmitterManager::~AEmitterManager()
{
	string managerConfigurePath = format("./{}Config.cfg", m_managerName);

	std::ofstream ofs(managerConfigurePath.c_str(), std::ios::binary);

	size_t length = m_emitters.size();
	SerializeHelper::SerializeElement<size_t>(ofs, length);

	for (auto& emitter : m_emitters)
	{
		const XMVECTOR& position = emitter->GetPosition();
		const XMVECTOR& angle = emitter->GetAngle();

		SerializeHelper::SerializeElement<XMVECTOR>(ofs, position);
		SerializeHelper::SerializeElement<XMVECTOR>(ofs, angle);
		emitter->Serialize(ofs);
	}
}

void AEmitterManager::LoadManager(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	string managerConfigurePath = format("./{}Config.cfg", m_managerName);
	std::ifstream ifs(managerConfigurePath.c_str(), std::ios::binary);
	if (ifs)
	{
		size_t length = SerializeHelper::DeserializeElement<size_t>(ifs);
		for (size_t idx = 0; idx < length; ++idx)
		{
			XMVECTOR position = SerializeHelper::DeserializeElement<XMVECTOR>(ifs);
			XMVECTOR angle  = SerializeHelper::DeserializeElement<XMVECTOR>(ifs);
			UINT emitterID = AddEmitter(position, angle, device, deviceContext);

			AEmitter* emitter = GetEmitter(emitterID);
			emitter->Deserialize(ifs);
			emitter->Initialize(device, deviceContext);
		}
	}
}

UINT AEmitterManager::IssueAvailableEmitterID()
{
	if (m_emitterIDQueue.empty()) { throw exception("No Emitter ID To Issue"); }

	UINT emitterID = m_emitterIDQueue.front();
	m_emitterIDQueue.pop();

	return emitterID;
}

void AEmitterManager::ReclaimEmitterID(UINT emitterID) noexcept
{
	ZeroMem(m_worldTransformCPU[emitterID]);
	AddWorldTransformChangedEmitterID(emitterID);

	ZeroMem(m_forcePropertyCPU[emitterID]);
	AddForceChangedEmitterID(emitterID);

	m_emitterIDQueue.push(emitterID);
}

void AEmitterManager::AddWorldTransformChangedEmitterID(UINT emitterID)
{
	m_worldTransformChangedEmitterIDs.emplace_back(emitterID);
}

void AEmitterManager::AddForceChangedEmitterID(UINT emitterID)
{
	m_forcePropertyChangedEmitterIDs.emplace_back(emitterID);
}

AEmitter* AEmitterManager::GetEmitter(UINT emitterID)
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

bool AEmitterManager::FindEmitterFromID(
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

void AEmitterManager::RemoveEmitter(UINT emitterID)
{
	vector<unique_ptr<AEmitter>>::iterator iter;
	if (FindEmitterFromID(emitterID, iter))
	{
		ReclaimEmitterID(emitterID);
		m_emitters.erase(iter);
	}
	else
	{
		throw exception("Remove Emitter With ID Failed");
	}
}

void AEmitterManager::SelectColorGPUInterpolater(UINT emitterID, UINT colorInterpolaterID, bool isColorGPUInterpolaterOn, EInterpolationMethod colorInterpolationMethod, IInterpolater<4>* colorInterpolater)
{
	AEmitter* particleEmitter = GetEmitter(emitterID);
	if (colorInterpolaterID == InterpPropertyNotSelect && isColorGPUInterpolaterOn)
	{
		switch (colorInterpolationMethod)
		{
		case EInterpolationMethod::Linear:
		{
			colorInterpolaterID = m_colorD1Dim4PorpertyManager->IssueAvailableInterpPropertyID();
			break;
		}
		case EInterpolationMethod::CubicSpline:
		case EInterpolationMethod::CatmullRom:
		{
			colorInterpolaterID = m_colorD3Dim4PorpertyManager->IssueAvailableInterpPropertyID();
			break;
		}
		}
		particleEmitter->SetColorInterpolaterID(colorInterpolaterID);
	}

	else if (colorInterpolaterID != InterpPropertyNotSelect && !isColorGPUInterpolaterOn)
	{
		particleEmitter->SetColorInterpolaterID(InterpPropertyNotSelect);
		switch (colorInterpolationMethod)
		{
		case EInterpolationMethod::Linear:
			m_colorD1Dim4PorpertyManager->ReclaimInterpPropertyID(colorInterpolaterID);
			break;
		case EInterpolationMethod::CubicSpline:
		case EInterpolationMethod::CatmullRom:
			m_colorD3Dim4PorpertyManager->ReclaimInterpPropertyID(colorInterpolaterID);
			break;
		}
	}
	else;
}

void AEmitterManager::UpdateColorGPUInterpolater(
	UINT emitterID,
	UINT colorInterpolaterID, 
	bool isColorGPUInterpolaterOn,
	float maxLife, 
	EInterpolationMethod colorInterpolationMethod, 
	IInterpolater<4>* colorInterpolater
)
{
	if (colorInterpolaterID != InterpPropertyNotSelect && isColorGPUInterpolaterOn)
	{
		switch (colorInterpolationMethod)
		{
		case EInterpolationMethod::Linear:
		{
			SInterpProperty<4, 2>* interpProperty = m_colorD1Dim4PorpertyManager->GetInterpProperty(colorInterpolaterID);
			interpProperty->UpdateInterpolaterProperty(
				static_cast<UINT>(colorInterpolationMethod),
				colorInterpolater->GetXProfilesAddress(), colorInterpolater->GetXProfilesCount(),
				colorInterpolater->GetCoefficientsAddress(), colorInterpolater->GetCoefficientsCount()
			);
			m_colorD1Dim4PorpertyManager->AddChangedEmitterInterpPropertyID(colorInterpolaterID);
			break;
		}
		case EInterpolationMethod::CubicSpline:
		case EInterpolationMethod::CatmullRom:
			SInterpProperty<4, 4>* interpProperty = m_colorD3Dim4PorpertyManager->GetInterpProperty(colorInterpolaterID);
			interpProperty->UpdateInterpolaterProperty(
				static_cast<UINT>(colorInterpolationMethod),
				colorInterpolater->GetXProfilesAddress(), colorInterpolater->GetXProfilesCount(),
				colorInterpolater->GetCoefficientsAddress(), colorInterpolater->GetCoefficientsCount()
			);
			m_colorD3Dim4PorpertyManager->AddChangedEmitterInterpPropertyID(colorInterpolaterID);
			break;
		}
	}
	UpdateColorGPUInterpolaterImpl(emitterID, colorInterpolaterID, isColorGPUInterpolaterOn, maxLife, colorInterpolationMethod, colorInterpolater);
}

void AEmitterManager::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	InitializeImpl(device, deviceContext);
	LoadManager(device, deviceContext);
}

void AEmitterManager::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	for (auto& particleEmitter : m_emitters)
	{
		particleEmitter->Update(deviceContext, dt);
	}
	UpdateImpl(deviceContext, dt);	
}

void AEmitterManager::AddInterpolaterInformChangedEmitterID(UINT emitterID)
{
	m_interpInformationChangedEmitterIDs.emplace_back(emitterID);
}

void AEmitterManager::InitializeImpl(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	D3D11_DRAW_INSTANCED_INDIRECT_ARGS drawIndirectArgs;
	ZeroMem(drawIndirectArgs);
	drawIndirectArgs.VertexCountPerInstance = 0;
	drawIndirectArgs.InstanceCount = 1;
	drawIndirectArgs.StartVertexLocation = NULL;
	drawIndirectArgs.StartInstanceLocation = NULL;

	const UINT particleMaxCount = m_emitterManagerPropertyCPU.particleMaxCount;

	m_emitterManagerPropertyGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_emitterManagerPropertyCPU));
	m_emitterManagerPropertyGPU->InitializeBuffer(device);

	m_totalParticles = make_unique<CStructuredBuffer>(static_cast<UINT>(sizeof(SParticle)), particleMaxCount, nullptr);
	m_deathIndexSet = make_unique<CAppendBuffer>(4, particleMaxCount, nullptr);

	m_totalParticles->InitializeBuffer(device);
	m_deathIndexSet->InitializeBuffer(device);

	CreateAliveIndexSet(device);

	m_drawIndirectBuffer = make_unique<CIndirectBuffer<D3D11_DRAW_INSTANCED_INDIRECT_ARGS>>(1, &drawIndirectArgs);
	m_drawIndirectBuffer->InitializeBuffer(device);

	m_dispatchIndirectStagingBuffer = make_unique<CDynamicBuffer>(4, 4, nullptr);
	m_dispatchIndirectStagingBuffer->InitializeBuffer(device);
	
	m_dispatchIndirectCalculatedBuffer = make_unique<CStructuredBuffer>(4, 4, nullptr);
	m_dispatchIndirectCalculatedBuffer->InitializeBuffer(device);

	m_dispatchIndirectBuffer = make_unique<CIndirectBuffer<D3D11_DISPATCH_INDIRECT_ARGS>>(1, nullptr);
	m_dispatchIndirectBuffer->InitializeBuffer(device);

	m_instancedWorldTransformGPU = make_unique<CDynamicBuffer>(
		static_cast<UINT>(sizeof(XMMATRIX)),
		m_maxEmitterCount,
		m_worldTransformCPU.data(),
		D3D11_BIND_VERTEX_BUFFER
	);

	m_worldTransformGPU = make_unique<CStructuredBuffer>(
		static_cast<UINT>(sizeof(XMMATRIX)),
		m_maxEmitterCount,
		m_worldTransformCPU.data()
	);

	m_forcePropertyGPU = make_unique<CStructuredBuffer>(
		static_cast<UINT>(sizeof(SEmitterForceProperty)),
		m_maxEmitterCount,
		m_forcePropertyCPU.data()
	);

	m_instancedWorldTransformGPU->InitializeBuffer(device);
	m_worldTransformGPU->InitializeBuffer(device);
	m_forcePropertyGPU->InitializeBuffer(device);

	m_colorD1Dim4PorpertyManager = make_unique<CGPUInterpPropertyManager<4, 2>>(m_maxEmitterCount);
	m_colorD3Dim4PorpertyManager = make_unique<CGPUInterpPropertyManager<4, 4>>(m_maxEmitterCount);

	m_colorD1Dim4PorpertyManager->Initialize(device, deviceContext);
	m_colorD3Dim4PorpertyManager->Initialize(device, deviceContext);

}

void AEmitterManager::UpdateImpl(ID3D11DeviceContext* deviceContext, float dt)
{
	if (m_isEmitterManagerPropertyChanged)
	{
		m_emitterManagerPropertyGPU->Stage(deviceContext);
		m_emitterManagerPropertyGPU->Upload(deviceContext);
		m_isEmitterManagerPropertyChanged = false;
	}

	UINT worldTransformChangedCount = static_cast<UINT>(m_worldTransformChangedEmitterIDs.size());
	if (worldTransformChangedCount > 0)
	{
		m_instancedWorldTransformGPU->StageNthElement(deviceContext, m_worldTransformChangedEmitterIDs.data(), worldTransformChangedCount);
		m_instancedWorldTransformGPU->UploadNthElement(deviceContext, m_worldTransformChangedEmitterIDs.data(), worldTransformChangedCount);

		for (const UINT changedId : m_worldTransformChangedEmitterIDs)
		{
			m_worldTransformCPU[changedId] = XMMatrixTranspose(m_worldTransformCPU[changedId]);
		}

		m_worldTransformGPU->StageNthElement(deviceContext, m_worldTransformChangedEmitterIDs.data(), worldTransformChangedCount);
		m_worldTransformGPU->UploadNthElement(deviceContext, m_worldTransformChangedEmitterIDs.data(), worldTransformChangedCount);

		for (const UINT changedId : m_worldTransformChangedEmitterIDs)
		{
			m_worldTransformCPU[changedId] = XMMatrixTranspose(m_worldTransformCPU[changedId]);
		}

		m_worldTransformChangedEmitterIDs.clear();
	}

	UINT forcePropertyChangedCount = static_cast<UINT>(m_forcePropertyChangedEmitterIDs.size());
	if (forcePropertyChangedCount > 0)
	{
		m_forcePropertyGPU->StageNthElement(deviceContext, m_forcePropertyChangedEmitterIDs.data(), forcePropertyChangedCount);
		m_forcePropertyGPU->UploadNthElement(deviceContext, m_forcePropertyChangedEmitterIDs.data(), forcePropertyChangedCount);
		m_forcePropertyChangedEmitterIDs.clear();
	}

	UINT interpInformationChangedCount = static_cast<UINT>(m_interpInformationChangedEmitterIDs.size());
	if (interpInformationChangedCount > 0)
	{
		m_emitterInterpInformationGPU->StageNthElement(deviceContext, m_interpInformationChangedEmitterIDs.data(), interpInformationChangedCount);
		m_emitterInterpInformationGPU->UploadNthElement(deviceContext, m_interpInformationChangedEmitterIDs.data(), interpInformationChangedCount);
		m_interpInformationChangedEmitterIDs.clear();
	}

	m_colorD1Dim4PorpertyManager->Update(deviceContext, dt);
	m_colorD3Dim4PorpertyManager->Update(deviceContext, dt);
}

void AEmitterManager::SourceParticles(ID3D11DeviceContext* deviceContext)
{
	const UINT emitterType = GetEmitterType();

	ID3D11ShaderResourceView* sourceSrvs[] = { m_worldTransformGPU->GetSRV() };
	ID3D11ShaderResourceView* sourceNullSrvs[] = { nullptr };

	ID3D11UnorderedAccessView* sourceUavs[] = {
		m_totalParticles->GetUAV(),
		m_deathIndexSet->GetUAV(),
		m_aliveIndexSet->GetUAV()
	};
	ID3D11UnorderedAccessView* sourceNullUavs[] = { nullptr, nullptr, nullptr };
	UINT initialValue[3] = { NULL,  static_cast<UINT>(-1), static_cast<UINT>(-1) };

	deviceContext->CSSetShaderResources(0, 1, sourceSrvs);
	deviceContext->CSSetUnorderedAccessViews(0, 3, sourceUavs, initialValue);

#pragma region 檬扁拳 家教
	CEmitterManagerCommonData::GParticleInitialSourceCS[emitterType]->SetShader(deviceContext);
	for (auto& emitter : m_emitters)
	{
		if (!emitter->IsSpawned())
		{
			CInitialSpawnProperty* initialSpawnProperty = emitter->GetInitialSpawnProperty();
			ID3D11Buffer* emitterInitialSourceBuffers[] = {
				m_emitterManagerPropertyGPU->GetBuffer(),
				emitter->GetEmitterPropertyBuffer(),
				initialSpawnProperty->GetInitialSpawnPropertyBuffer()
			};
			ID3D11Buffer* emitterInitialSourceNullBuffers[] = { nullptr, nullptr, nullptr };

			deviceContext->CSSetConstantBuffers(2, 3, emitterInitialSourceBuffers);
			const UINT dispatchX = UINT(ceil(initialSpawnProperty->GetInitialParticleCount() / LocalThreadCount));
			deviceContext->Dispatch(dispatchX, 1, 1);
			deviceContext->CSSetConstantBuffers(2, 3, emitterInitialSourceNullBuffers);
			emitter->SetSpawned(true);
		}
	}
	CEmitterManagerCommonData::GParticleInitialSourceCS[emitterType]->ResetShader(deviceContext);
#pragma endregion

#pragma region 繁鸥烙 家教
	CEmitterManagerCommonData::GParticleRuntimeSourceCS[emitterType]->SetShader(deviceContext);
	for (auto& emitter : m_emitters)
	{
		CEmitterUpdateProperty* emitterUpdateProperty = emitter->GetEmitterUpdateProperty();
		ARuntimeSpawnProperty* runtimeSpawnProperty = emitter->GetRuntimeSpawnProperty();

		ID3D11Buffer* emitterRuntimeSourceBuffers[] = {
			m_emitterManagerPropertyGPU->GetBuffer(),
			emitter->GetEmitterPropertyBuffer(),
			runtimeSpawnProperty->GetParticleSpawnPropertyBuffer()
		};

		ID3D11Buffer* emitterRuntimeSourceNullBuffers[] = { nullptr, nullptr, nullptr };

		deviceContext->CSSetConstantBuffers(2, 3, emitterRuntimeSourceBuffers);
		const UINT dispatchX = emitterUpdateProperty->GetSpawnCount();
		deviceContext->Dispatch(dispatchX, 1, 1);
		deviceContext->CSSetConstantBuffers(2, 3, emitterRuntimeSourceNullBuffers);
	}
	CEmitterManagerCommonData::GParticleRuntimeSourceCS[emitterType]->ResetShader(deviceContext);
#pragma endregion

	deviceContext->CSSetUnorderedAccessViews(0, 3, sourceNullUavs, nullptr);
	deviceContext->CSSetShaderResources(0, 1, sourceNullSrvs);
}

void AEmitterManager::CalculateIndirectArgs(ID3D11DeviceContext* deviceContext)
{
	deviceContext->CopyStructureCount(m_dispatchIndirectStagingBuffer->GetBuffer(), NULL, m_aliveIndexSet->GetUAV());

	CEmitterManagerCommonData::GCalcualteIndirectArgCS->SetShader(deviceContext);
	ID3D11Buffer* stagingCB = m_dispatchIndirectStagingBuffer->GetBuffer();
	ID3D11Buffer* stagingNullCB = nullptr;
	ID3D11UnorderedAccessView* stagingUAV = m_dispatchIndirectCalculatedBuffer->GetUAV();
	ID3D11UnorderedAccessView* stagingNullUAV = nullptr;

	deviceContext->CSSetConstantBuffers(2, 1, &stagingCB);
	deviceContext->CSSetUnorderedAccessViews(0, 1, &stagingUAV, nullptr);
	deviceContext->Dispatch(1, 1, 1);
	deviceContext->CSSetConstantBuffers(2, 1, &stagingNullCB);
	deviceContext->CSSetUnorderedAccessViews(0, 1, &stagingNullUAV, nullptr);

	CEmitterManagerCommonData::GCalcualteIndirectArgCS->ResetShader(deviceContext);

	deviceContext->CopyResource(m_dispatchIndirectBuffer->GetBuffer(), m_dispatchIndirectCalculatedBuffer->GetBuffer());
	deviceContext->CopyStructureCount(m_drawIndirectBuffer->GetBuffer(), NULL, m_aliveIndexSet->GetUAV());
}

void AEmitterManager::CalculateForces(ID3D11DeviceContext* deviceContext)
{
	const UINT emitterType = GetEmitterType();

	ID3D11Buffer* simulateCBs[] = { m_dispatchIndirectStagingBuffer->GetBuffer() };
	ID3D11Buffer* simulateNullCBs[] = { nullptr };

	ID3D11ShaderResourceView* simulateSrvs[] = { m_worldTransformGPU->GetSRV(), m_forcePropertyGPU->GetSRV()};
	ID3D11ShaderResourceView* simulateNullSrvs[] = { nullptr, nullptr};

	ID3D11UnorderedAccessView* simulateUavs[] = { m_totalParticles->GetUAV(), m_aliveIndexSet->GetUAV() };
	ID3D11UnorderedAccessView* simulateNullUavs[] = { nullptr, nullptr };
	UINT initialValue[2] = { NULL,  static_cast<UINT>(-1) };

	CEmitterManagerCommonData::GCaculateParticleForceCS[emitterType]->SetShader(deviceContext);

	deviceContext->CSSetConstantBuffers(2, 1, simulateCBs);
	deviceContext->CSSetShaderResources(0, 2, simulateSrvs);
	deviceContext->CSSetUnorderedAccessViews(0, 2, simulateUavs, initialValue);

	deviceContext->DispatchIndirect(m_dispatchIndirectBuffer->GetBuffer(), NULL);

	deviceContext->CSSetConstantBuffers(2, 1, simulateNullCBs);
	deviceContext->CSSetShaderResources(0, 2, simulateNullSrvs);
	deviceContext->CSSetUnorderedAccessViews(0, 2, simulateNullUavs, nullptr);

	CEmitterManagerCommonData::GCaculateParticleForceCS[emitterType]->ResetShader(deviceContext);
}

void AEmitterManager::DrawEmitters(ID3D11DeviceContext* deviceContext)
{
	ID3D11Buffer* vertexBuffers[] = {
		CEmitterManagerCommonData::GEmitterPositionBuffer->GetBuffer(),
		m_instancedWorldTransformGPU->GetBuffer()
	};
	ID3D11Buffer* vertexNullBuffers[] = { nullptr, nullptr };
	ID3D11Buffer* indexBuffer = CEmitterManagerCommonData::GEmitterIndexBuffer->GetBuffer();

	UINT strides[] = { static_cast<UINT>(sizeof(XMFLOAT3)), static_cast<UINT>(sizeof(XMMATRIX)) };
	UINT nullStrides[] = { NULL, NULL };
	UINT offsets[] = { 0, 0 };
	UINT nullOffsets[] = { NULL, NULL };

	UINT emitterTypeIndex = GetEmitterType();

	CEmitterManagerCommonData::GDrawEmitterPSO[emitterTypeIndex]->ApplyPSO(deviceContext);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	deviceContext->IASetVertexBuffers(0, 2, vertexBuffers, strides, offsets);
	deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, NULL);
	deviceContext->DrawIndexedInstanced(
		static_cast<UINT>(CEmitterManagerCommonData::GEmitterBoxIndices.size()),
		m_maxEmitterCount,
		NULL, NULL, NULL
	);
	deviceContext->IASetVertexBuffers(0, 2, vertexNullBuffers, nullStrides, nullOffsets);
	deviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, NULL);

	CEmitterManagerCommonData::GDrawEmitterPSO[emitterTypeIndex]->RemovePSO(deviceContext);
}
