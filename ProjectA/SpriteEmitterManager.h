#pragma once
#include "AEmitterManager.h"

#define MaxSpriteEmitterCount 250

class SpriteEmitterManager : public AEmitterManager
{
private:
	SpriteEmitterManager(
		UINT maxEmitterCount,
		UINT maxParticleCount
	);
	SpriteEmitterManager(const SpriteEmitterManager&) = delete;
	SpriteEmitterManager& operator=(const SpriteEmitterManager&) = delete;
	~SpriteEmitterManager() override = default;


public:
	static SpriteEmitterManager& GetSpriteEmitterManager();

protected:
	virtual UINT GetEmitterType() const noexcept override { return static_cast<UINT>(EEmitterType::SpriteEmitter); }

protected:
	virtual void ReclaimEmitterID(UINT emitterID) noexcept override;

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
	std::unique_ptr<CGPUInterpPropertyManager<2, 2>> m_spriteSizeD1Dim4PorpertyManager;
	std::unique_ptr<CGPUInterpPropertyManager<2, 4>> m_spriteSizeD3Dim4PorpertyManager;

protected:
	virtual void InitializeImpl(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;

public:
	virtual void InitializeAliveFlag(ID3D11DeviceContext* deviceContext) override;
	virtual void DrawParticles(ID3D11DeviceContext* deviceContext) override;
};

// ParticleSpawnEmitter와 
// SpriteSpawnEmitter에 functoin으로 
// OnInterpolaterChanged와 같은 식으로
// m_colorD1Dim4PorpertyManager, m_colorD3Dim4PorpertyManager로 처리