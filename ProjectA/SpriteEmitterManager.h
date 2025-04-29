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
		EInterpolationMethod spriteIndexInterpolationMethod,
		IInterpolater<1>* spriteIndexInterpolater
	);

protected:
	virtual void InitializeImpl(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void UpdateImpl(ID3D11DeviceContext* deviceContext, float dt) override;

public:
	virtual void InitializeAliveFlag(ID3D11DeviceContext* deviceContext) override;
	virtual void DrawParticles(ID3D11DeviceContext* deviceContext) override;
};