#pragma once
#include "AEmitterManager.h"

#define MaxParticleEmitterCount 250

class ParticleEmitterManager : public AEmitterManager
{
private:
	ParticleEmitterManager(
		UINT maxEmitterCount,
		UINT maxParticleCount
	);
	ParticleEmitterManager(const ParticleEmitterManager&) = delete;
	ParticleEmitterManager& operator=(const ParticleEmitterManager&) = delete;
	~ParticleEmitterManager() override = default;

public:
	static ParticleEmitterManager& GetParticleEmitterManager();

protected:
	virtual UINT GetEmitterType() const noexcept override { return static_cast<UINT>(EEmitterType::ParticleEmitter); }

protected:
	virtual void ReclaimEmitterID(UINT emitterID) noexcept override;

protected:
	std::vector<SParticleInterpInformation> m_emitterInterpInformationCPU;

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

