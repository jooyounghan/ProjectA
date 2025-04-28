#include "SpriteEmitterManager.h"

#include "EmitterManagerCommonData.h"
#include "InitialSpawnProperty.h"
#include "EmitterUpdateProperty.h"
#include "ARuntimeSpawnProperty.h"

#include "ComputeShader.h"
#include "GraphicsPSOObject.h"
#include "ConstantBuffer.h"

#include "SpriteEmitter.h"
#include "GPUInterpPropertyManager.h"
#include "MacroUtilities.h"

using namespace std;
using namespace DirectX;
using namespace D3D11;

SpriteEmitterManager::SpriteEmitterManager(
	UINT maxEmitterCount,
	UINT maxParticleCount
)
	: AEmitterManager("SpriteEmitterManager", static_cast<UINT>(EEmitterType::SpriteEmitter), maxEmitterCount, maxParticleCount)
{
	SSpriteInterpInformation spriteInterpInformation;
	AutoZeroMemory(spriteInterpInformation);

	m_emitterInterpInformationCPU.resize(m_maxEmitterCount, spriteInterpInformation);
	m_forcePropertyChangedEmitterIDs.reserve(m_maxEmitterCount);
}

SpriteEmitterManager& SpriteEmitterManager::GetSpriteEmitterManager()
{
	static SpriteEmitterManager spriteEmitterManager(MaxSpriteEmitterCount, MaxParticleCount);
	return spriteEmitterManager;
}

void SpriteEmitterManager::ReclaimEmitterID(UINT emitterID) noexcept
{
	AutoZeroMemory(m_emitterInterpInformationCPU[emitterID]);
	AddInterpolaterInformChangedEmitterID(emitterID);

	AEmitterManager::ReclaimEmitterID(emitterID);
}

UINT SpriteEmitterManager::AddEmitter(DirectX::XMVECTOR position, DirectX::XMVECTOR angle, ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	UINT spriteEmitterID = IssueAvailableEmitterID();
	unique_ptr<SpriteEmitter> spriteEmitter = make_unique<SpriteEmitter>(
		spriteEmitterID,
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
		},
		[this](UINT emitterID, UINT spriteSizeInterpolaterID, bool isSpriteSizeGPUInterpolaterOn, EInterpolationMethod spriteSizeInterpolationMethod, IInterpolater<2>* spriteSizeInterpolater)
		{
			SelectSpriteSizeGPUInterpolater(emitterID, spriteSizeInterpolaterID, isSpriteSizeGPUInterpolaterOn, spriteSizeInterpolationMethod, spriteSizeInterpolater);
		},
		[this](UINT emitterID, UINT spriteSizeInterpolaterID, bool isSpriteSizeGPUInterpolaterOn, float maxLife, EInterpolationMethod spriteSizeInterpolationMethod, IInterpolater<2>* spriteSizeInterpolater)
		{
			UpdateSpriteSizeGPUInterpolater(emitterID, spriteSizeInterpolaterID, isSpriteSizeGPUInterpolaterOn, maxLife, spriteSizeInterpolationMethod, spriteSizeInterpolater);
		}
	);

	SpriteEmitter* emitter = spriteEmitter.get();
	m_emitters.emplace_back(std::move(spriteEmitter));
	emitter->Initialize(device, deviceContext);
	return spriteEmitterID;

}

void SpriteEmitterManager::UpdateColorGPUInterpolaterImpl(
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

void SpriteEmitterManager::SelectSpriteSizeGPUInterpolater(
	UINT emitterID, 
	UINT spriteSizeInterpolaterID, 
	bool isSpriteSizeGPUInterpolaterOn, 
	EInterpolationMethod spriteSizeInterpolationMethod, 
	IInterpolater<2>* spriteSizeInterpolater
)
{
	SpriteEmitter* spriteEmitter = reinterpret_cast<SpriteEmitter*>(GetEmitter(emitterID));
	if (spriteSizeInterpolaterID == InterpPropertyNotSelect && isSpriteSizeGPUInterpolaterOn)
	{
		switch (spriteSizeInterpolationMethod)
		{
		case EInterpolationMethod::Linear:
		{
			spriteSizeInterpolaterID = m_spriteSizeD1Dim2PorpertyManager->IssueAvailableInterpPropertyID();
			break;
		}
		case EInterpolationMethod::CubicSpline:
		case EInterpolationMethod::CatmullRom:
		{
			spriteSizeInterpolaterID = m_spriteSizeD1Dim2PorpertyManager->IssueAvailableInterpPropertyID();
			break;
		}
		}
		spriteEmitter->SetSpriteSizeInterpolaterID(spriteSizeInterpolaterID);
	}
	else if (spriteSizeInterpolaterID != InterpPropertyNotSelect && !isSpriteSizeGPUInterpolaterOn)
	{
		spriteEmitter->SetSpriteSizeInterpolaterID(InterpPropertyNotSelect);
		switch (spriteSizeInterpolationMethod)
		{
		case EInterpolationMethod::Linear:
			m_spriteSizeD1Dim2PorpertyManager->ReclaimInterpPropertyID(spriteSizeInterpolaterID);
			break;
		case EInterpolationMethod::CubicSpline:
		case EInterpolationMethod::CatmullRom:
			m_spriteSizeD1Dim2PorpertyManager->ReclaimInterpPropertyID(spriteSizeInterpolaterID);
			break;
		}
	}
	else;

}

void SpriteEmitterManager::UpdateSpriteSizeGPUInterpolater(
	UINT emitterID,
	UINT spriteSizeInterpolaterID,
	bool isSpriteSizeGPUInterpolaterOn,
	float maxLife,
	EInterpolationMethod spriteSizeInterpolationMethod, 
	IInterpolater<2>* spriteSizeInterpolater
)
{
	if (spriteSizeInterpolaterID != InterpPropertyNotSelect && isSpriteSizeGPUInterpolaterOn)
	{
		switch (spriteSizeInterpolationMethod)
		{
		case EInterpolationMethod::Linear:
		{
			SInterpProperty<2, 2>* interpProperty = m_spriteSizeD1Dim2PorpertyManager->GetInterpProperty(spriteSizeInterpolaterID);
			interpProperty->UpdateInterpolaterProperty(
				static_cast<UINT>(spriteSizeInterpolationMethod),
				spriteSizeInterpolater->GetXProfilesAddress(), spriteSizeInterpolater->GetXProfilesCount(),
				spriteSizeInterpolater->GetCoefficientsAddress(), spriteSizeInterpolater->GetCoefficientsCount()
			);
			m_spriteSizeD1Dim2PorpertyManager->AddChangedEmitterInterpPropertyID(spriteSizeInterpolaterID);
			break;
		}
		case EInterpolationMethod::CubicSpline:
		case EInterpolationMethod::CatmullRom:
			SInterpProperty<2, 4>* interpProperty = m_spriteSizeD3Dim2PorpertyManager->GetInterpProperty(spriteSizeInterpolaterID);
			interpProperty->UpdateInterpolaterProperty(
				static_cast<UINT>(spriteSizeInterpolationMethod),
				spriteSizeInterpolater->GetXProfilesAddress(), spriteSizeInterpolater->GetXProfilesCount(),
				spriteSizeInterpolater->GetCoefficientsAddress(), spriteSizeInterpolater->GetCoefficientsCount()
			);
			m_spriteSizeD3Dim2PorpertyManager->AddChangedEmitterInterpPropertyID(spriteSizeInterpolaterID);
			break;
		}
	}

	m_emitterInterpInformationCPU[emitterID].maxLife = maxLife;
	m_emitterInterpInformationCPU[emitterID].spriteSizeInterpolaterID = spriteSizeInterpolaterID;
	m_emitterInterpInformationCPU[emitterID].spriteSizeInterpolaterDegree = spriteSizeInterpolater->GetDegree();
	AddInterpolaterInformChangedEmitterID(emitterID);
}

void SpriteEmitterManager::InitializeImpl(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	AEmitterManager::InitializeImpl(device, deviceContext);

	m_emitterInterpInformationGPU = make_unique<CStructuredBuffer>(
		static_cast<UINT>(sizeof(SSpriteInterpInformation)),
		m_maxEmitterCount,
		m_emitterInterpInformationCPU.data()
	);
	m_emitterInterpInformationGPU->InitializeBuffer(device);

	m_spriteSizeD1Dim2PorpertyManager = make_unique<CGPUInterpPropertyManager<2, 2>>(m_maxEmitterCount);
	m_spriteSizeD3Dim2PorpertyManager = make_unique<CGPUInterpPropertyManager<2, 4>>(m_maxEmitterCount);

	m_spriteSizeD1Dim2PorpertyManager->Initialize(device, deviceContext);
	m_spriteSizeD3Dim2PorpertyManager->Initialize(device, deviceContext);
}

void SpriteEmitterManager::UpdateImpl(ID3D11DeviceContext* deviceContext, float dt)
{
	AEmitterManager::UpdateImpl(deviceContext, dt);

	m_spriteSizeD1Dim2PorpertyManager->Update(deviceContext, dt);
	m_spriteSizeD3Dim2PorpertyManager->Update(deviceContext, dt);
}

void SpriteEmitterManager::InitializeAliveFlag(ID3D11DeviceContext* deviceContext)
{
	ID3D11Buffer* initializeCBs[] = { m_emitterManagerPropertyGPU->GetBuffer() };
	ID3D11Buffer* initializeNullCBs[] = { nullptr };

	ID3D11ShaderResourceView* initializeSRVs[] = {
		m_emitterInterpInformationGPU->GetSRV(),
		m_colorD1Dim4PorpertyManager->GetGPUInterpPropertySRV(),
		m_colorD3Dim4PorpertyManager->GetGPUInterpPropertySRV(),
		m_spriteSizeD1Dim2PorpertyManager->GetGPUInterpPropertySRV(),
		m_spriteSizeD3Dim2PorpertyManager->GetGPUInterpPropertySRV()
	};
	ID3D11ShaderResourceView* initializeNullSRVs[] = { nullptr, nullptr, nullptr, nullptr, nullptr };

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
	deviceContext->CSSetShaderResources(0, 5, initializeSRVs);
	deviceContext->CSSetUnorderedAccessViews(0, 3, initializeUavs, initDeathParticleCount);
	static const UINT dispatchX = static_cast<UINT>(ceil(m_emitterManagerPropertyCPU.particleMaxCount / LocalThreadCount));
	deviceContext->Dispatch(dispatchX, 1, 1);
	deviceContext->CSSetConstantBuffers(2, 1, initializeNullCBs);
	deviceContext->CSSetShaderResources(0, 5, initializeNullSRVs);
	deviceContext->CSSetUnorderedAccessViews(0, 3, initializeNullUavs, initDeathParticleCount);
}

void SpriteEmitterManager::DrawParticles(ID3D11DeviceContext* deviceContext)
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
