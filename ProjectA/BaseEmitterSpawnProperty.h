#pragma once
#include "IProperty.h"
#include "DynamicBuffer.h"
#include "ShapedVectorSelector.h"

#include <d3d11.h>

class BaseEmitterSpawnProperty : public IProperty
{
public:
	BaseEmitterSpawnProperty();
	virtual ~BaseEmitterSpawnProperty() = default;

protected:
	bool m_isSpawned = false;

public:
	inline bool IsSpawned() const noexcept { return m_isSpawned; }
	inline void SetSpawned(bool isSpawned) { m_isSpawned = isSpawned; }

protected:
	struct alignas(16)
	{
		SShapedVectorProperty shapedVectorSelector;
		UINT initialParticleCount;
		float initialParticleLife;
	} m_emitterSpawnPropertyCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_emitterSpawnPropertyGPU;
	bool m_isEmitterSpawnPropertyChanged = false;

public:
	void SetShapedVectorProperty(const SShapedVectorProperty& shapedVectorSelector);
	void SetInitialParticleCount(UINT initialParticleCount);
	void SetInitialParticleLife(float initialParticleLife);

public:
	inline ID3D11Buffer* GetEmitterSpawnPropertyBuffer() const noexcept { return m_emitterSpawnPropertyGPU->GetBuffer(); }
	inline UINT GetInitialParticleCount() const noexcept { return m_emitterSpawnPropertyCPU.initialParticleCount; }

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

public:
	virtual void DrawPropertyUI() override;
};

