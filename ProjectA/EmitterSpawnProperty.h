#pragma once
#include "Updatable.h"
#include "DynamicBuffer.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

class CEmitterSpawnProperty : public IUpdatable
{
public:
	CEmitterSpawnProperty(
		const DirectX::XMFLOAT2& minInitRadians,
		const DirectX::XMFLOAT2& maxInitRadians,
		const DirectX::XMFLOAT2& minMaxRadius,
		UINT initialParticleCount
	);
public:
	~CEmitterSpawnProperty() = default;

protected:
	bool m_isSpawned = false;

public:
	inline bool IsSpawned() const noexcept { return m_isSpawned; }
	inline void SetSpawned() { m_isSpawned = true; }

protected:
	struct  
	{
		const DirectX::XMFLOAT2 minInitRadians;
		const DirectX::XMFLOAT2 maxInitRadians;
		const DirectX::XMFLOAT2 minMaxRadius;
		const UINT initialParticleCount;
		const UINT dummy;
	} m_emitterSpawnPropertyCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_emitterSpawnPropertyGPU;

public:
	inline ID3D11Buffer* GetEmitterSpawnPropertyBuffer() const noexcept { return m_emitterSpawnPropertyGPU->GetBuffer(); }
	inline UINT GetInitialParticleCount() const noexcept { return m_emitterSpawnPropertyCPU.initialParticleCount; }

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;
};

