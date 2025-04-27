#pragma once
#include "IProperty.h"
#include "ISerializable.h"
#include "ShapedVectorProperty.h"

#include "Interpolater.h"
#include "ShapedVectorSelector.h"
#include "ControlPointGridView.h"
#include "InterpolaterSelector.h"
#include "GPUInterpPropertyManager.h"

#include <vector>
#include <memory>

namespace D3D11
{
	class CDynamicBuffer;
}

class ARuntimeSpawnProperty : public IProperty, public ISerializable
{
public:
	ARuntimeSpawnProperty(uint32_t maxEmitterCount);
	~ARuntimeSpawnProperty() override = default;

protected:
	virtual void OnInterpolateInformationChagned() = 0;

protected:
	float m_currentLifeTime;

protected:
	struct  
	{
		union 
		{
			SShapedVectorProperty shapedPositionVectorProperty;
			struct
			{
				char padding1[88];
				float maxLife;
				float padding2;
			};
		};
		union
		{
			SShapedVectorProperty shapedSpeedVectorProperty;
			struct
			{
				char padding3[88];
				DirectX::XMFLOAT2 size;
			};
		};
		DirectX::XMVECTOR color;
	} m_baseParticleSpawnPropertyCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_baseParticleSpawnPropertyGPU;
	bool m_isParticleSpawnPropertyChanged;

public:
	ID3D11Buffer* GetParticleSpawnPropertyBuffer() const noexcept;

protected:
	EShapedVector m_positionShapedVector;
	std::unique_ptr<CShapedVectorSelector> m_positionShapedVectorSelector;
	DirectX::XMFLOAT3 m_positionOrigin;
	DirectX::XMVECTOR m_positionUpVector;

protected:
	EShapedVector m_speedShapedVector;
	std::unique_ptr<CShapedVectorSelector> m_speedShapedVectorSelector;
	DirectX::XMFLOAT3 m_speedOrigin;
	DirectX::XMVECTOR m_speedUpVector;

protected:
	SControlPoint<4> m_colorInitControlPoint;
	SControlPoint<4> m_colorFinalControlPoint;
	std::vector<SControlPoint<4>> m_colorControlPoints;
	EInterpolationMethod m_colorInterpolationMethod;
	std::unique_ptr<IInterpolater<4>> m_colorInterpolater;

protected:
	bool m_checkGPUColorInterpolater;

protected:
	std::unique_ptr<CControlPointGridView<4>> m_colorControlPointGridView;
	std::unique_ptr<CInterpolaterSelectPlotter<4>> m_colorInterpolationSelectPlotter;

protected:
	virtual void AdjustControlPointsFromLife();

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

public:
	virtual void DrawPropertyUI() override;

protected:
	virtual void DrawPropertyUIImpl() override;

public:
	virtual void Serialize(std::ofstream& ofs) override;
	virtual void Deserialize(std::ifstream& ifs) override;
};

