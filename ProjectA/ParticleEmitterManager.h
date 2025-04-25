#pragma once
#include "AEmitterManager.h"
#include "ParticleInterpInformation.h"
#include "EmitterTypeDefinition.h"

class ParticleEmitterManager : public AEmitterManager
{
private:
	ParticleEmitterManager(UINT maxEmitterCount);
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
	std::vector<UINT> m_interpInformationChangedEmitterIDs;

public:
	std::unique_ptr<D3D11::CStructuredBuffer> m_emitterInterpInformationGPU;

public:
	virtual void AddInterpolaterInformChangedEmitterID(UINT emitterID) override;

public:
	virtual UINT AddEmitter(
		DirectX::XMVECTOR position,
		DirectX::XMVECTOR angle,
		ID3D11Device* device, 
		ID3D11DeviceContext* deviceContext
	) override;

protected:
	virtual void InitializeImpl(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void UpdateImpl(ID3D11DeviceContext* deviceContext, float dt) override;

public:
	virtual void InitializeAliveFlag(ID3D11DeviceContext* deviceContext) override;
	virtual void DrawParticles(ID3D11DeviceContext* deviceContext) override;
};

