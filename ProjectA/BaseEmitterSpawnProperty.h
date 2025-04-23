#pragma once
#include "IProperty.h"
#include "ShapedVectorProperty.h"

#include <memory>

namespace D3D11
{
	class CDynamicBuffer;
}
class ShapedVectorSelector;

class BaseEmitterSpawnProperty : public IProperty
{
public:
	BaseEmitterSpawnProperty();
	~BaseEmitterSpawnProperty() override = default;

protected:
	struct
	{
		union
		{
			SShapedVectorProperty shapedPositionVectorProperty;
			struct
			{
				char padding1[88];
				UINT initialParticleCount;
				float initialParticleLife;
			};
		};
		SShapedVectorProperty shapedSpeedVectorProperty;
		DirectX::XMVECTOR color;
	} m_emitterSpawnPropertyCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_emitterSpawnPropertyGPU;
	bool m_isEmitterSpawnPropertyChanged = false;

protected:
	bool m_isImmortal = false;

protected:
	EShapedVector m_positionShapedVector;
	std::unique_ptr<ShapedVectorSelector> m_positionShapedVectorSelector;

protected:
	EShapedVector m_speedShapedVector;
	std::unique_ptr<ShapedVectorSelector> m_speedShapedVectorSelector;

public:
	ID3D11Buffer* GetEmitterSpawnPropertyBuffer() const noexcept;
	inline UINT GetInitialParticleCount() const noexcept { return m_emitterSpawnPropertyCPU.initialParticleCount; }

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

public:
	virtual void DrawPropertyUI() override;
};

