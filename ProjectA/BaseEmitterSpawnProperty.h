#pragma once
#include "IProperty.h"
#include "ISerializable.h"
#include "ShapedVectorProperty.h"

#include <memory>

namespace D3D11
{
	class CDynamicBuffer;
}
class CShapedVectorSelector;

class CBaseEmitterSpawnProperty : public IProperty, public ISerializable
{
public:
	CBaseEmitterSpawnProperty();
	~CBaseEmitterSpawnProperty() override = default;

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
		union
		{
			SShapedVectorProperty shapedSpeedVectorProperty;
			struct
			{
				char padding2[88];
				float padding3[2];
			};
		};
		DirectX::XMVECTOR color;
	} m_emitterSpawnPropertyCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_emitterSpawnPropertyGPU;
	bool m_isEmitterSpawnPropertyChanged;

protected:
	bool m_isImmortal = false;

protected:
	EShapedVector m_positionShapedVector;
	std::unique_ptr<CShapedVectorSelector> m_positionShapedVectorSelector;

protected:
	EShapedVector m_speedShapedVector;
	std::unique_ptr<CShapedVectorSelector> m_speedShapedVectorSelector;

public:
	ID3D11Buffer* GetEmitterSpawnPropertyBuffer() const noexcept;
	inline UINT GetInitialParticleCount() const noexcept { return m_emitterSpawnPropertyCPU.initialParticleCount; }

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

public:
	virtual void DrawPropertyUI() override;

public:
	virtual void Serialize(std::ofstream& ofs) override;
	virtual void Deserialize(std::ifstream& ifs) override;
};

