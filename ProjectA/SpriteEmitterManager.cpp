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

#include "stb_image_resize2.h"

using namespace std;
using namespace DirectX;
using namespace D3D11;

SpriteEmitterManager::SpriteEmitterManager(
	UINT maxEmitterCount,
	UINT maxParticleCount
)
	: AEmitterManager("SpriteEmitterManager", static_cast<UINT>(EEmitterType::SpriteEmitter), maxEmitterCount, maxParticleCount),
	m_spriteTextureWidth(MaxSpriteTextureWidth),
	m_spriteTextureHeight(MaxSpriteTextureHeight)
{
	SSpriteInterpInformation spriteInterpInformation;
	ZeroMem(spriteInterpInformation);

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
	ZeroMem(m_emitterInterpInformationCPU[emitterID]);
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
}

void SpriteEmitterManager::DrawParticles(ID3D11DeviceContext* deviceContext)
{
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
}
