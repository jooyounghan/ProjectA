#pragma once
#include "IProperty.h"
#include "DynamicBuffer.h"
#include "ShapedVectorSelector.h"
#include "InterpolationSelector.h"

template<uint32_t Dim>
class IInterpolater;

template<uint32_t Dim>
class ControlPointGridView;

template<uint32_t Dim>
class InterpolationSelectPlotter;

enum class EInterpolationMethod;

class BaseParticleSpawnProperty : public APropertyHasLoopTime
{
public:
	BaseParticleSpawnProperty(float& emitterCurrentTime, float& loopTime);
	virtual ~BaseParticleSpawnProperty() = default;

protected:
	float& m_emitterCurrentTime;
	float m_lastLoopTime;

protected:
	struct  
	{
		union 
		{
			SShapedVectorProperty shapedPositionVectorProperty;
			struct
			{
				char padding1[88];
				DirectX::XMFLOAT2 minMaxLifeTime;
			};
		};
		union
		{
			SShapedVectorProperty shapedSpeedVectorProperty;
			struct
			{
				char padding2[88];
				DirectX::XMFLOAT2 padding3;
			};
		};
		DirectX::XMVECTOR color;
	} m_baseParticleSpawnPropertyCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_baseParticleSpawnPropertyGPU;

public:
	inline ID3D11Buffer* GetParticleSpawnPropertyBuffer() const noexcept { return m_baseParticleSpawnPropertyGPU->GetBuffer(); }

protected:

protected:
	EShapedVector m_positionShapedVector;
	std::unique_ptr<ShapedVectorSelector> m_positionShapedVectorSelector;

protected:
	EShapedVector m_speedShapedVector;
	std::unique_ptr<ShapedVectorSelector> m_speedShapedVectorSelector;

protected:
	SControlPoint<2> m_lifeInitControlPoint;
	SControlPoint<2> m_lifeFinalControlPoint;
	std::vector<SControlPoint<2>> m_lifeControlPoints;
	EInterpolationMethod m_lifeInterpolationMethod;
	std::unique_ptr<IInterpolater<2>> m_lifeInterpolater;

protected:
	std::unique_ptr<ControlPointGridView<2>> m_lifeControlPointGridView;
	std::unique_ptr<InterpolationSelectPlotter<2>> m_lifeInterpolationSelectPlotter;

protected:
	SControlPoint<3> m_colorInitControlPoint;
	SControlPoint<3> m_colorFinalControlPoint;
	std::vector<SControlPoint<3>> m_colorControlPoints;
	EInterpolationMethod m_colorInterpolationMethod;
	std::unique_ptr<IInterpolater<3>> m_colorInterpolater;

protected:
	std::unique_ptr<ControlPointGridView<3>> m_colorControlPointGridView;
	std::unique_ptr<InterpolationSelectPlotter<3>> m_colorInterpolationSelectPlotter;

protected:
	virtual void AdjustControlPointsFromLoopTime() override;

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

public:
	virtual void DrawPropertyUI() override;
};

