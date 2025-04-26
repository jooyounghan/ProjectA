#pragma once
#include "AEmitterManager.h"

#define MaxSpriteEmitterCount 250

class SpriteEmitterManager : public AEmitterManager
{
private:
	SpriteEmitterManager(UINT maxEmitterCount);
	SpriteEmitterManager(const SpriteEmitterManager&) = delete;
	SpriteEmitterManager& operator=(const SpriteEmitterManager&) = delete;
	~SpriteEmitterManager() override = default;


public:
	static SpriteEmitterManager& GetSpriteEmitterManager();

protected:
	virtual UINT GetEmitterType() const noexcept override { return static_cast<UINT>(EEmitterType::ParticleEmitter); }

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
	virtual void InitializeImpl(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;

public:
	virtual void InitializeAliveFlag(ID3D11DeviceContext* deviceContext) override;
	virtual void DrawParticles(ID3D11DeviceContext* deviceContext) override;
};

