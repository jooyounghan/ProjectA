#pragma once
#include "IProperty.h"
#include "ShapedVectorProperty.h"
#include "InterpolaterStructure.h"

#include <vector>
#include <memory>
#include <functional>

namespace D3D11
{
	class CDynamicBuffer;
}

template<uint32_t Dim, bool GPUInterpolateOn>
class IInterpolater;

template<uint32_t Dim>
class ControlPointGridView;

template<uint32_t Dim, bool GPUInterpolateOn>
class InterpolaterSelectPlotter;

class ShapedVectorSelector;

class BaseParticleSpawnProperty : public IProperty
{
public:
	BaseParticleSpawnProperty(const std::function<void(uint32_t, uint32_t)>& colorInterpolationChangedHandler);
	~BaseParticleSpawnProperty() override = default;

protected:
	std::function<void(uint32_t, uint32_t)> m_onColorInterpolationChanged;

protected:
	struct  
	{
		union 
		{
			SShapedVectorProperty shapedPositionVectorProperty;
			struct
			{
				char padding1[88];
				float life;
				float padding2;
			};
		};
		union
		{
			SShapedVectorProperty shapedSpeedVectorProperty;
			struct
			{
				char padding3[88];
				float padding4[2];
			};
		};
	} m_baseParticleSpawnPropertyCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_baseParticleSpawnPropertyGPU;
	bool m_isParticleSpawnPropertyChanged;

public:
	ID3D11Buffer* GetParticleSpawnPropertyBuffer() const noexcept;

protected:
	EShapedVector m_positionShapedVector;
	std::unique_ptr<ShapedVectorSelector> m_positionShapedVectorSelector;

protected:
	EShapedVector m_speedShapedVector;
	std::unique_ptr<ShapedVectorSelector> m_speedShapedVectorSelector;

protected:
	float m_lastParticleLife;

protected:
	SControlPoint<4> m_colorInitControlPoint;
	SControlPoint<4> m_colorFinalControlPoint;
	std::vector<SControlPoint<4>> m_colorControlPoints;
	EInterpolationMethod m_colorInterpolationMethod;
	std::unique_ptr<IInterpolater<4, true>> m_colorInterpolater;

protected:
	std::unique_ptr<ControlPointGridView<4>> m_colorControlPointGridView;
	std::unique_ptr<InterpolaterSelectPlotter<4, true>> m_colorInterpolationSelectPlotter;

protected:
	void AdjustControlPointsFromLife();

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

public:
	virtual void DrawPropertyUI() override;
};

