#pragma once
#include "AEmitterManager.h"

class CBloomFilm;
class CMotionBlurFilm;

class ParticleEmitterManager : public AEmitterManager
{
public:
	ParticleEmitterManager(
		UINT effectWidth,
		UINT effectHeight,
		UINT maxEmitterCount,
		UINT maxParticleCount
	);
	~ParticleEmitterManager() override = default;

protected:
	virtual UINT GetEmitterType() const noexcept override { return static_cast<UINT>(EEmitterType::ParticleEmitter); }
	virtual void ReclaimEmitterID(UINT emitterID) noexcept override;
	virtual void CreateAliveIndexSet(ID3D11Device* device) override;

protected:
	std::unique_ptr<CBloomFilm> m_bloomFilm;
	std::unique_ptr<CMotionBlurFilm> m_motionBlurFilm;

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
	virtual void UpdateColorGPUInterpolaterImpl(
		UINT emitterID,
		UINT colorInterpolaterID,
		bool isColorGPUInterpolaterOn,
		float maxLife,
		EInterpolationMethod colorInterpolationMethod,
		IInterpolater<4>* colorInterpolater
	) override;

protected:
	virtual void InitializeImpl(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;

public:
	virtual void InitializeAliveFlag(CShotFilm* shotFilm, ID3D11DeviceContext* deviceContext) override;
	virtual void FinalizeParticles(ID3D11DeviceContext* deviceContext);
	virtual void DrawParticles(CShotFilm* shotFilm, ID3D11DeviceContext* deviceContext) override;
};

