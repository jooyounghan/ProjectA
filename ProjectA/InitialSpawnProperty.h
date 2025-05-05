#pragma once
#include "IDrawable.h"
#include "IUpdatable.h"
#include "ISerializable.h"
#include "ShapedVectorProperty.h"

#include <memory>

namespace D3D11
{
	class CDynamicBuffer;
}
class CShapedVectorSelector;

class CInitialSpawnProperty : public IDrawable, public IUpdatable, public ISerializable
{
public:
	CInitialSpawnProperty();
	~CInitialSpawnProperty() override = default;

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
				DirectX::XMFLOAT2 xyScale;
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
	DirectX::XMFLOAT3 m_positionOrigin; 
	DirectX::XMVECTOR m_positionUpVector; 
	float m_positionCenterAngle;

protected:
	EShapedVector m_speedShapedVector;
	std::unique_ptr<CShapedVectorSelector> m_speedShapedVectorSelector;
	DirectX::XMFLOAT3 m_speedOrigin;
	DirectX::XMVECTOR m_speedUpVector;
	float m_speedCenterAngle;

public:
	ID3D11Buffer* GetInitialSpawnPropertyBuffer() const noexcept;
	inline UINT GetInitialParticleCount() const noexcept { return m_emitterSpawnPropertyCPU.initialParticleCount; }

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

public:
	virtual void DrawUI() override;

protected:
	virtual void DrawUIImpl() override;

public:
	virtual void Serialize(std::ofstream& ofs) override;
	virtual void Deserialize(std::ifstream& ifs) override;
};

