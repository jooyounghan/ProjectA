#pragma once
#include "IProperty.h"
#include "DynamicBuffer.h"
#include "ShapedVectorSelector.h"

#include <memory>

class BaseEmitterSpawnProperty : public IProperty
{
public:
	BaseEmitterSpawnProperty();
	virtual ~BaseEmitterSpawnProperty() = default;

protected:
	struct alignas(16)
	{
		SShapedVectorProperty shapedPositionVectorProperty;
		UINT initialParticleCount;
		float initialParticleLife;
		DirectX::XMFLOAT2 dummy;
	} m_emitterSpawnPropertyCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_emitterSpawnPropertyGPU;
	bool m_isEmitterSpawnPropertyChanged = false;

protected:
	DirectX::XMFLOAT3 m_origin;
	DirectX::XMVECTOR m_upVector;

public:
	inline ID3D11Buffer* GetEmitterSpawnPropertyBuffer() const noexcept { return m_emitterSpawnPropertyGPU->GetBuffer(); }
	inline UINT GetInitialParticleCount() const noexcept { return m_emitterSpawnPropertyCPU.initialParticleCount; }

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

public:
	virtual void DrawPropertyUI() override;
};

