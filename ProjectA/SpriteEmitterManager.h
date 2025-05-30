#pragma once
#include "AEmitterManager.h"
#include "Texture2DInstance.h"
#include "SRVOption.h"

#define MaxSpriteTextureWidth 1000
#define MaxSpriteTextureHeight 1000

struct SSpriteAliveIndex
{
	UINT index;
	UINT depth;
};

struct SPrefixSumDesciptor
{
	UINT aggregate;
	UINT statusFlag;	/* X : 0, A : 1, P : 2*/
	UINT exclusivePrefix;
	UINT inclusivePrefix;
};

class CBloomFilm;

class SpriteEmitterManager : public AEmitterManager
{
public:
	SpriteEmitterManager(
		UINT effectWidth,
		UINT effectHeight,
		UINT maxEmitterCount,
		UINT maxParticleCount
	);
	~SpriteEmitterManager() override = default;

protected:
	virtual UINT GetEmitterType() const noexcept override { return static_cast<UINT>(EEmitterType::SpriteEmitter); }

protected:
	virtual void ReclaimEmitterID(UINT emitterID) noexcept override;

protected:
	struct
	{
		UINT sortBitOffset;
		UINT padding1;
		UINT padding2;
		UINT padding3;
	} m_radixSortPropertyCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_radixSortPropertyGPU;

protected:
	std::unique_ptr<D3D11::CAppendBuffer> m_sortedAliveIndexSet;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_aliveIndexUAV;
	Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_sortedAliveIndexUAV;

protected:
	std::unique_ptr<D3D11::CStructuredBuffer> m_prefixLocalHistogramDispatchArgsBuffer;
	std::unique_ptr<D3D11::CIndirectBuffer<D3D11_DISPATCH_INDIRECT_ARGS>> m_prefixLocalHistogramDispatchIndirectBuffer;

protected:
	std::unique_ptr<D3D11::CStructuredBuffer> m_localHistogram;
	std::unique_ptr<D3D11::CStructuredBuffer> m_localPrefixSumDescriptors;
	std::unique_ptr<D3D11::CStructuredBuffer> m_globlaHistogram;

protected:
	virtual void CreateAliveIndexSet(ID3D11Device* device) override;

protected:
	std::vector<SSpriteInterpInformation> m_emitterInterpInformationCPU;

public:
	virtual UINT AddEmitter(
		DirectX::XMVECTOR position,
		DirectX::XMVECTOR angle,
		ID3D11Device* device,
		ID3D11DeviceContext* deviceContext
	) override;

protected:
	std::unique_ptr<CGPUInterpPropertyManager<2, 2>> m_spriteSizeD1Dim2PorpertyManager;
	std::unique_ptr<CGPUInterpPropertyManager<2, 4>> m_spriteSizeD3Dim2PorpertyManager;

protected:
	std::unique_ptr<CGPUInterpPropertyManager<1, 2>> m_spriteIndexD1Dim1PorpertyManager;
	std::unique_ptr<CGPUInterpPropertyManager<1, 4>> m_spriteIndexD3Dim1PorpertyManager;
	
protected:
	virtual void UpdateColorGPUInterpolaterImpl(
		UINT emitterID,
		UINT colorInterpolaterID,
		bool isColorGPUInterpolaterOn,
		float maxLife,
		EInterpolationMethod colorInterpolationMethod,
		IInterpolater<4>* colorInterpolater
	) override;

protected:
	void SelectSpriteSizeGPUInterpolater(
		UINT emitterID,
		UINT spriteSizeInterpolaterID,
		bool isSpriteSizeGPUInterpolaterOn,
		EInterpolationMethod spriteSizeInterpolationMethod,
		IInterpolater<2>* spriteSizeInterpolater
	);

protected:
	void UpdateSpriteSizeGPUInterpolater(
		UINT emitterID,
		UINT spriteSizeInterpolaterID, 
		bool isSpriteSizeGPUInterpolaterOn,
		float maxLife, 
		EInterpolationMethod spriteSizeInterpolationMethod, 
		IInterpolater<2>* spriteSizeInterpolater
	);

protected:
	void SelectSpriteIndexGPUInterpolater(
		UINT emitterID,
		UINT spriteIndexInterpolaterID,
		bool isSpriteIndexGPUInterpolaterOn,
		EInterpolationMethod spriteIndexInterpolationMethod,
		IInterpolater<1>* spriteIndexInterpolater
	);

protected:
	void UpdateSpriteIndexGPUInterpolater(
		UINT emitterID,
		UINT spriteIndexInterpolaterID,
		bool isSpriteIndexGPUInterpolaterOn,
		float maxLife,
		const DirectX::XMFLOAT2& spriteTextureCount,
		EInterpolationMethod spriteIndexInterpolationMethod,
		IInterpolater<1>* spriteIndexInterpolater
	);

protected:
	void UpdateSpriteTexture(
		UINT emitterID, 
		uint8_t* loadedBuffer, 
		UINT width, 
		UINT height, 
		UINT channel, 
		ID3D11Device* device,
		ID3D11DeviceContext* deviceContext
	);

protected:
	UINT m_spriteTextureWidth;
	UINT m_spriteTextureHeight;
	std::unique_ptr<Texture2DInstance<D3D11::SRVOption>> m_spriteTextureArray;

protected:
	virtual void InitializeImpl(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void UpdateImpl(ID3D11DeviceContext* deviceContext, float dt) override;

public:
	virtual void InitializeAliveFlag(CShotFilm* shotFilm, CBaseFilm* normalFilm, ID3D11DeviceContext* deviceContext) override;
	virtual void CalculateIndirectArgs(ID3D11DeviceContext* deviceContext) override;
	virtual void FinalizeParticles(ID3D11DeviceContext* deviceContext);
	virtual void DrawParticles(CShotFilm* shotFilm, ID3D11DeviceContext* deviceContext) override;
};