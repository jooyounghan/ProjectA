#include "SpriteEmitterManager.h"

#include "EmitterManagerCommonData.h"
#include "InitialSpawnProperty.h"
#include "EmitterUpdateProperty.h"
#include "RuntimeSpawnProperty.h"

#include "ComputeShader.h"
#include "GraphicsPSOObject.h"
#include "ConstantBuffer.h"

#include "SpriteEmitter.h"
#include "GPUInterpPropertyManager.h"
#include "MacroUtilities.h"

#include "ShotFilm.h"
#include "BloomFilm.h"

#include "stb_image_resize2.h"

#define RadixBinCount (1 << RadixBitCount)

using namespace std;
using namespace DirectX;
using namespace D3D11;

SpriteEmitterManager::SpriteEmitterManager(
	UINT effectWidth,
	UINT effectHeight,
	UINT maxEmitterCount,
	UINT maxParticleCount
)
	: AEmitterManager("SpriteEmitterManager", maxEmitterCount, maxParticleCount),
	m_bloomFilm(make_unique<CBloomFilm>(3, 1.f, effectWidth, effectHeight, DXGI_FORMAT_R8G8B8A8_UNORM, 1, 4)),
	m_spriteTextureWidth(MaxSpriteTextureWidth),
	m_spriteTextureHeight(MaxSpriteTextureHeight),
	m_sortBitOffset(m_emitterManagerPropertyCPU.padding1)
{
	SSpriteInterpInformation spriteInterpInformation;
	ZeroMem(spriteInterpInformation);

	m_emitterInterpInformationCPU.resize(m_maxEmitterCount, spriteInterpInformation);
	m_forcePropertyChangedEmitterIDs.reserve(m_maxEmitterCount);
}

void SpriteEmitterManager::ReclaimEmitterID(UINT emitterID) noexcept
{
	ZeroMem(m_emitterInterpInformationCPU[emitterID]);
	AddInterpolaterInformChangedEmitterID(emitterID);

	AEmitterManager::ReclaimEmitterID(emitterID);
}

void SpriteEmitterManager::CreateAliveIndexSet(ID3D11Device* device)
{
	const UINT particleMaxCount = m_emitterManagerPropertyCPU.particleMaxCount;
	m_aliveIndexSet = make_unique<CAppendBuffer>(
		static_cast<UINT>(sizeof(SSpriteAliveIndex)), 
		particleMaxCount, nullptr
	);
	m_aliveIndexSet->InitializeBuffer(device);

	m_sortedAliveIndexSet = make_unique<CAppendBuffer>(
		static_cast<UINT>(sizeof(SSpriteAliveIndex)),
		particleMaxCount, nullptr
	);
	m_sortedAliveIndexSet->InitializeBuffer(device);

	m_localHistogramSet = make_unique<CStructuredBuffer>(
		static_cast<UINT>(sizeof(SRadixHistogram)), 
		static_cast<UINT>(ceil(particleMaxCount / LocalThreadCount)), nullptr
	);
	m_localHistogramSet->InitializeBuffer(device);

	m_localPrefixSumStatus = make_unique<CStructuredBuffer>(
		static_cast<UINT>(sizeof(SPrefixDesciptor)),
		static_cast<UINT>(ceil(ceil(particleMaxCount / LocalThreadCount) / LocalThreadCount)) * RadixBinCount,
		nullptr
	);
	m_localPrefixSumStatus->InitializeBuffer(device);

	m_globalHistogramSet = make_unique<CStructuredBuffer>(4, RadixBinCount, nullptr);
	m_globalHistogramSet->InitializeBuffer(device);

	m_globalPrefixSumStatus = make_unique<CStructuredBuffer>(
		static_cast<UINT>(sizeof(SPrefixDesciptor)),
		static_cast<UINT>(UINT(ceil(32.f / RadixBitCount))), nullptr
	);
	m_globalPrefixSumStatus->InitializeBuffer(device);

	D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
	ZeroMem(uavDesc);
	uavDesc.Format = DXGI_FORMAT_UNKNOWN;
	uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
	uavDesc.Buffer.NumElements = particleMaxCount;
	device->CreateUnorderedAccessView(m_aliveIndexSet->GetBuffer(), &uavDesc, m_aliveIndexRWSet.GetAddressOf());
	device->CreateUnorderedAccessView(m_sortedAliveIndexSet->GetBuffer(), &uavDesc, m_sortedAliveIndexRWSet.GetAddressOf());
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
		},
		[this](UINT emitterID, UINT spriteIndexInterpolaterID, bool isSpriteIndexGPUInterpolaterOn, EInterpolationMethod spriteIndexInterpolationMethod, IInterpolater<1>* spriteIndexInterpolater)
		{
			SelectSpriteIndexGPUInterpolater(emitterID, spriteIndexInterpolaterID, isSpriteIndexGPUInterpolaterOn, spriteIndexInterpolationMethod, spriteIndexInterpolater);
		},
		[this](UINT emitterID, UINT spriteIndexInterpolaterID, bool isSpriteIndexGPUInterpolaterOn, float maxLife, UINT spriteTextureCount, EInterpolationMethod spriteIndexInterpolationMethod, IInterpolater<1>* spriteIndexInterpolater)
		{
			UpdateSpriteIndexGPUInterpolater(emitterID, spriteIndexInterpolaterID, isSpriteIndexGPUInterpolaterOn, maxLife, spriteTextureCount, spriteIndexInterpolationMethod, spriteIndexInterpolater);
		},
		[device, deviceContext, this](UINT emitterID, uint8_t* loadedBuffer, UINT width, UINT height, UINT channel)
		{
			UpdateSpriteTexture(emitterID, loadedBuffer, width, height, channel, device, deviceContext);
		}
	);

	spriteEmitter->CreateProperty();
	m_emitters.emplace_back(std::move(spriteEmitter));
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

void SpriteEmitterManager::SelectSpriteIndexGPUInterpolater(
	UINT emitterID, 
	UINT spriteIndexInterpolaterID, 
	bool isSpriteIndexGPUInterpolaterOn, 
	EInterpolationMethod spriteIndexInterpolationMethod, 
	IInterpolater<1>* spriteIndexInterpolater
)
{
	SpriteEmitter* spriteEmitter = reinterpret_cast<SpriteEmitter*>(GetEmitter(emitterID));
	if (spriteIndexInterpolaterID == InterpPropertyNotSelect && isSpriteIndexGPUInterpolaterOn)
	{
		switch (spriteIndexInterpolationMethod)
		{
		case EInterpolationMethod::Linear:
		{
			spriteIndexInterpolaterID = m_spriteIndexD1Dim1PorpertyManager->IssueAvailableInterpPropertyID();
			break;
		}
		case EInterpolationMethod::CubicSpline:
		case EInterpolationMethod::CatmullRom:
		{
			spriteIndexInterpolaterID = m_spriteIndexD3Dim1PorpertyManager->IssueAvailableInterpPropertyID();
			break;
		}
		}
		spriteEmitter->SetSpriteIndexInterpolaterID(spriteIndexInterpolaterID);
	}
	else if (spriteIndexInterpolaterID != InterpPropertyNotSelect && !isSpriteIndexGPUInterpolaterOn)
	{
		spriteEmitter->SetSpriteIndexInterpolaterID(InterpPropertyNotSelect);
		switch (spriteIndexInterpolationMethod)
		{
		case EInterpolationMethod::Linear:
			m_spriteIndexD1Dim1PorpertyManager->ReclaimInterpPropertyID(spriteIndexInterpolaterID);
			break;
		case EInterpolationMethod::CubicSpline:
		case EInterpolationMethod::CatmullRom:
			m_spriteIndexD3Dim1PorpertyManager->ReclaimInterpPropertyID(spriteIndexInterpolaterID);
			break;
		}
	}
	else;
}

void SpriteEmitterManager::UpdateSpriteIndexGPUInterpolater(
	UINT emitterID, 
	UINT spriteIndexInterpolaterID, 
	bool isSpriteIndexGPUInterpolaterOn, 
	float maxLife,
	UINT spriteTextureCount,
	EInterpolationMethod spriteIndexInterpolationMethod, 
	IInterpolater<1>* spriteIndexInterpolater
)
{
	if (spriteIndexInterpolaterID != InterpPropertyNotSelect && isSpriteIndexGPUInterpolaterOn)
	{
		switch (spriteIndexInterpolationMethod)
		{
		case EInterpolationMethod::Linear:
		{
			SInterpProperty<1, 2>* interpProperty = m_spriteIndexD1Dim1PorpertyManager->GetInterpProperty(spriteIndexInterpolaterID);
			interpProperty->UpdateInterpolaterProperty(
				static_cast<UINT>(spriteIndexInterpolationMethod),
				spriteIndexInterpolater->GetXProfilesAddress(), spriteIndexInterpolater->GetXProfilesCount(),
				spriteIndexInterpolater->GetCoefficientsAddress(), spriteIndexInterpolater->GetCoefficientsCount()
			);
			m_spriteIndexD1Dim1PorpertyManager->AddChangedEmitterInterpPropertyID(spriteIndexInterpolaterID);
			break;
		}
		case EInterpolationMethod::CubicSpline:
		case EInterpolationMethod::CatmullRom:
			SInterpProperty<1, 4>* interpProperty = m_spriteIndexD3Dim1PorpertyManager->GetInterpProperty(spriteIndexInterpolaterID);
			interpProperty->UpdateInterpolaterProperty(
				static_cast<UINT>(spriteIndexInterpolationMethod),
				spriteIndexInterpolater->GetXProfilesAddress(), spriteIndexInterpolater->GetXProfilesCount(),
				spriteIndexInterpolater->GetCoefficientsAddress(), spriteIndexInterpolater->GetCoefficientsCount()
			);
			m_spriteIndexD3Dim1PorpertyManager->AddChangedEmitterInterpPropertyID(spriteIndexInterpolaterID);
			break;
		}
	}

	m_emitterInterpInformationCPU[emitterID].maxLife = maxLife;
	m_emitterInterpInformationCPU[emitterID].spriteTextureCount = spriteTextureCount;
	m_emitterInterpInformationCPU[emitterID].spriteIndexInterpolaterID = spriteIndexInterpolaterID;
	m_emitterInterpInformationCPU[emitterID].spriteIndexInterpolaterDegree = spriteIndexInterpolater->GetDegree();
	AddInterpolaterInformChangedEmitterID(emitterID);
}

void SpriteEmitterManager::UpdateSpriteTexture(
	UINT emitterID, 
	uint8_t* loadedBuffer, 
	UINT width, 
	UINT height, 
	UINT channel, 
	ID3D11Device* device,
	ID3D11DeviceContext* deviceContext
)
{
	uint8_t* rescaledBuffer = new uint8_t[width * height * channel];

	stbir_resize_uint8_linear(
		loadedBuffer, width, height, width * channel,
		rescaledBuffer,
		static_cast<int>(m_spriteTextureWidth),
		static_cast<int>(m_spriteTextureHeight),
		m_spriteTextureWidth * 4,
		stbir_pixel_layout::STBIR_RGBA
	);

	const UINT rowPitch = m_spriteTextureWidth * channel;
	const UINT mipLevel = 0;
	const UINT subresourceIndex = D3D11CalcSubresource(mipLevel, emitterID, 1);

	D3D11_BOX box = { 0, 0, 0, m_spriteTextureWidth, m_spriteTextureHeight, 1 };

	deviceContext->UpdateSubresource(
		m_spriteTextureArray->GetTexture2D(),
		subresourceIndex,
		&box,
		rescaledBuffer,
		rowPitch,
		0
	);

	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> updatedSpriteTextureSRV;
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	ZeroMem(srvDesc);
	srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;

	srvDesc.Texture2DArray.MostDetailedMip = 0;
	srvDesc.Texture2DArray.MipLevels = 1;
	srvDesc.Texture2DArray.FirstArraySlice = emitterID;
	srvDesc.Texture2DArray.ArraySize = 1;

	HRESULT hr = device->CreateShaderResourceView(m_spriteTextureArray->GetTexture2D(), &srvDesc, updatedSpriteTextureSRV.GetAddressOf());
	if (FAILED(hr)) { throw exception("CreateShaderResourceView For Sprite Texture Failed"); }

	SpriteEmitter* spriteEmitter = reinterpret_cast<SpriteEmitter*>(GetEmitter(emitterID));
	spriteEmitter->SetSpriteTextureSRV(move(updatedSpriteTextureSRV));

	delete[] rescaledBuffer;
}

void SpriteEmitterManager::InitializeImpl(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	AEmitterManager::InitializeImpl(device, deviceContext);

	m_dispatchRadixIndirectCalculatedBuffer = make_unique<CStructuredBuffer>(4, 4, nullptr);
	m_dispatchRadixIndirectCalculatedBuffer->InitializeBuffer(device);

	m_dispatchRadixIndirectBuffer = make_unique<CIndirectBuffer<D3D11_DISPATCH_INDIRECT_ARGS>>(1, nullptr);
	m_dispatchRadixIndirectBuffer->InitializeBuffer(device);

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

	m_spriteIndexD1Dim1PorpertyManager = make_unique< CGPUInterpPropertyManager<1, 2>>(m_maxEmitterCount);
	m_spriteIndexD3Dim1PorpertyManager = make_unique< CGPUInterpPropertyManager<1, 4>>(m_maxEmitterCount);

	m_spriteIndexD1Dim1PorpertyManager->Initialize(device, deviceContext);
	m_spriteIndexD3Dim1PorpertyManager->Initialize(device, deviceContext);

	m_spriteTextureArray = make_unique<Texture2DInstance<SRVOption>>(
		m_spriteTextureWidth, m_spriteTextureHeight, 
		m_maxEmitterCount, 1, NULL, NULL, 
		D3D11_USAGE_DEFAULT, DXGI_FORMAT_R8G8B8A8_UNORM_SRGB, 1, 4
	);
	m_spriteTextureArray->InitializeByOption(device, deviceContext);

	m_bloomFilm->Initialize(device, deviceContext);
}

void SpriteEmitterManager::UpdateImpl(ID3D11DeviceContext* deviceContext, float dt)
{
	AEmitterManager::UpdateImpl(deviceContext, dt);

	m_spriteSizeD1Dim2PorpertyManager->Update(deviceContext, dt);
	m_spriteSizeD3Dim2PorpertyManager->Update(deviceContext, dt);

	m_spriteIndexD1Dim1PorpertyManager->Update(deviceContext, dt);
	m_spriteIndexD3Dim1PorpertyManager->Update(deviceContext, dt);
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
		m_spriteSizeD3Dim2PorpertyManager->GetGPUInterpPropertySRV(),
		m_spriteIndexD1Dim1PorpertyManager->GetGPUInterpPropertySRV(),
		m_spriteIndexD3Dim1PorpertyManager->GetGPUInterpPropertySRV()
	};
	ID3D11ShaderResourceView* initializeNullSRVs[] = { nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr };

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
	deviceContext->CSSetShaderResources(0, 7, initializeSRVs);
	deviceContext->CSSetUnorderedAccessViews(0, 3, initializeUavs, initDeathParticleCount);
	static const UINT dispatchX = static_cast<UINT>(ceil(m_emitterManagerPropertyCPU.particleMaxCount / LocalThreadCount));
	deviceContext->Dispatch(dispatchX, 1, 1);
	deviceContext->CSSetConstantBuffers(2, 1, initializeNullCBs);
	deviceContext->CSSetShaderResources(0, 7, initializeNullSRVs);
	deviceContext->CSSetUnorderedAccessViews(0, 3, initializeNullUavs, initDeathParticleCount);

	CEmitterManagerCommonData::GInitializeParticleSetCS[emitterTypeIndex]->ResetShader(deviceContext);
}

void SpriteEmitterManager::CalculateIndirectArgs(ID3D11DeviceContext* deviceContext)
{
	AEmitterManager::CalculateIndirectArgs(deviceContext);

	CEmitterManagerCommonData::GCalcualteRadixIndirectArgCS->SetShader(deviceContext);

	ID3D11Buffer* stagingCB = m_dispatchIndirectStagingBuffer->GetBuffer();
	ID3D11Buffer* stagingNullCB = nullptr;
	ID3D11UnorderedAccessView* stagingUAV = m_dispatchRadixIndirectCalculatedBuffer->GetUAV();
	ID3D11UnorderedAccessView* stagingNullUAV = nullptr;

	deviceContext->CSSetConstantBuffers(2, 1, &stagingCB);
	deviceContext->CSSetUnorderedAccessViews(0, 1, &stagingUAV, nullptr);
	deviceContext->Dispatch(1, 1, 1);
	deviceContext->CSSetConstantBuffers(2, 1, &stagingNullCB);
	deviceContext->CSSetUnorderedAccessViews(0, 1, &stagingNullUAV, nullptr);

	CEmitterManagerCommonData::GCalcualteRadixIndirectArgCS->ResetShader(deviceContext);

	deviceContext->CopyResource(m_dispatchRadixIndirectBuffer->GetBuffer(), m_dispatchRadixIndirectCalculatedBuffer->GetBuffer());
}

void SpriteEmitterManager::FinalizeParticles(ID3D11DeviceContext* deviceContext)
{
	UINT clearValues[4] = { 0, 0, 0, 0 };

	ID3D11Buffer* finalizeCBs[] = { m_emitterManagerPropertyGPU->GetBuffer(), m_dispatchIndirectStagingBuffer->GetBuffer() };
	ID3D11Buffer* finalizeNullCBs[] = { nullptr, nullptr };

	ID3D11UnorderedAccessView* finalizeNullUavs[] = { nullptr, nullptr, nullptr, nullptr };
	UINT finalizeInitUavCount[] = { NULL, NULL, NULL, NULL };

	deviceContext->CSSetConstantBuffers(2, 2, finalizeCBs);
	UINT radixPathCount = UINT(ceil(32.f / RadixBitCount));


	for (UINT idx = 0; idx < radixPathCount; ++idx)
	{
		deviceContext->ClearUnorderedAccessViewUint(m_localHistogramSet->GetUAV(), clearValues);
		deviceContext->ClearUnorderedAccessViewUint(m_localPrefixSumStatus->GetUAV(), clearValues);
		deviceContext->ClearUnorderedAccessViewUint(m_globalHistogramSet->GetUAV(), clearValues);
		deviceContext->ClearUnorderedAccessViewUint(m_globalPrefixSumStatus->GetUAV(), clearValues);

		m_sortBitOffset = idx * RadixBitCount;

		m_emitterManagerPropertyGPU->Stage(deviceContext);
		m_emitterManagerPropertyGPU->Upload(deviceContext);

		ID3D11UnorderedAccessView* setRadixHistogramUavs[] = {
			m_aliveIndexRWSet.Get(),
			m_localHistogramSet->GetUAV(),
			m_globalHistogramSet->GetUAV()
		};

		CEmitterManagerCommonData::GSpriteSetRadixHistogramCS->SetShader(deviceContext);
		deviceContext->CSSetUnorderedAccessViews(0, 3, setRadixHistogramUavs, finalizeInitUavCount);
		deviceContext->DispatchIndirect(m_dispatchIndirectBuffer->GetBuffer(), NULL);
		deviceContext->CSSetUnorderedAccessViews(0, 3, finalizeNullUavs, finalizeInitUavCount);
		CEmitterManagerCommonData::GSpriteSetRadixHistogramCS->ResetShader(deviceContext);


		ID3D11UnorderedAccessView* setGlobalOffsetUavs[] = {
			m_aliveIndexRWSet.Get(),
			m_localHistogramSet->GetUAV(),
			m_localPrefixSumStatus->GetUAV(),
		};

		CEmitterManagerCommonData::GSpriteSetGlobalOffsetCS->SetShader(deviceContext);
		deviceContext->CSSetUnorderedAccessViews(0, 3, setGlobalOffsetUavs, finalizeInitUavCount);
		deviceContext->DispatchIndirect(m_dispatchRadixIndirectBuffer->GetBuffer(), NULL);
		deviceContext->CSSetUnorderedAccessViews(0, 3, finalizeNullUavs, finalizeInitUavCount);
		CEmitterManagerCommonData::GSpriteSetGlobalOffsetCS->ResetShader(deviceContext);

		ID3D11UnorderedAccessView* prefixSumRadixUavs[] = {
			m_globalHistogramSet->GetUAV(),
			m_globalPrefixSumStatus->GetUAV()
		};

		CEmitterManagerCommonData::GSpritePrefixSumRadixCS->SetShader(deviceContext);
		deviceContext->CSSetUnorderedAccessViews(0, 2, prefixSumRadixUavs, finalizeInitUavCount);
		deviceContext->Dispatch(radixPathCount, 1, 1);
		deviceContext->CSSetUnorderedAccessViews(0, 2, finalizeNullUavs, finalizeInitUavCount);
		CEmitterManagerCommonData::GSpritePrefixSumRadixCS->ResetShader(deviceContext);

		ID3D11UnorderedAccessView* sortingUavs[] = {
			m_aliveIndexRWSet.Get(),
			m_sortedAliveIndexRWSet.Get(),
			m_localHistogramSet->GetUAV(),
			m_globalHistogramSet->GetUAV()
		};

		CEmitterManagerCommonData::GSpriteSortingCS->SetShader(deviceContext);
		deviceContext->CSSetUnorderedAccessViews(0, 4, sortingUavs, finalizeInitUavCount);
		deviceContext->DispatchIndirect(m_dispatchIndirectBuffer->GetBuffer(), NULL);
		deviceContext->CSSetUnorderedAccessViews(0, 4, finalizeNullUavs, finalizeInitUavCount);
		CEmitterManagerCommonData::GSpriteSortingCS->ResetShader(deviceContext);

		m_aliveIndexRWSet.Swap(m_sortedAliveIndexRWSet);
	}

	deviceContext->CSSetConstantBuffers(2, 2, finalizeNullCBs);
}

void SpriteEmitterManager::DrawParticles(CShotFilm* shotFilm, ID3D11DeviceContext* deviceContext)
{
	m_bloomFilm->ClearFilm(deviceContext);

	ID3D11RenderTargetView* rtvs[] = { shotFilm->GetFilmRTV(), m_bloomFilm->GetFilmRTV() };
	ID3D11DepthStencilView* dsv = shotFilm->GetFilmDSV();
	D3D11_VIEWPORT viewports[] = { shotFilm->GetFilmViewPort(), m_bloomFilm->GetFilmViewPort() };

	deviceContext->OMSetRenderTargets(2, rtvs, dsv);
	deviceContext->RSSetViewports(2, viewports);

	ID3D11ShaderResourceView* vertexSrvs[] = {
		m_totalParticles->GetSRV(),
		m_aliveIndexSet->GetSRV()
	};
	ID3D11ShaderResourceView* vertexNullSrvs[] = { nullptr, nullptr };
	ID3D11ShaderResourceView* pixelSrvs[] = { m_emitterInterpInformationGPU->GetSRV(), m_spriteTextureArray->GetSRV() };
	ID3D11ShaderResourceView* pixelNullSrvs[] = { nullptr, nullptr };

	UINT emitterTypeIndex = GetEmitterType();

	const float blendColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	CEmitterManagerCommonData::GDrawParticlePSO[emitterTypeIndex]->ApplyPSO(deviceContext, blendColor, 0xFFFFFFFF);
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	deviceContext->VSSetShaderResources(0, 2, vertexSrvs);
	deviceContext->PSSetShaderResources(0, 2, pixelSrvs);

	deviceContext->DrawInstancedIndirect(m_drawIndirectBuffer->GetBuffer(), NULL);

	deviceContext->VSSetShaderResources(0, 2, vertexNullSrvs);
	deviceContext->PSSetShaderResources(0, 2, pixelNullSrvs);
	CEmitterManagerCommonData::GDrawParticlePSO[emitterTypeIndex]->RemovePSO(deviceContext);

	m_bloomFilm->Develop(deviceContext);
	m_bloomFilm->Blend(deviceContext, shotFilm);
}
