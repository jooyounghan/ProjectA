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
class CControlPointGridView;

template<uint32_t Dim, bool GPUInterpolateOn>
class CInterpolaterSelectPlotter;

class CShapedVectorSelector;

class CBaseParticleSpawnProperty : public IProperty
{
public:
	CBaseParticleSpawnProperty(
		const std::function<void(float)>& lifeChangedHandler,
		const std::function<void(uint32_t, uint32_t)>& colorInterpolationChangedHandler
	);
	~CBaseParticleSpawnProperty() override = default;

protected:
	std::function<void(float)> m_onLifeChanged;
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
	std::unique_ptr<CShapedVectorSelector> m_positionShapedVectorSelector;

protected:
	EShapedVector m_speedShapedVector;
	std::unique_ptr<CShapedVectorSelector> m_speedShapedVectorSelector;

protected:
	float m_lastParticleLife;

protected:
	SControlPoint<4> m_colorInitControlPoint;
	SControlPoint<4> m_colorFinalControlPoint;
	std::vector<SControlPoint<4>> m_colorControlPoints;
	EInterpolationMethod m_colorInterpolationMethod;
	std::unique_ptr<IInterpolater<4, true>> m_colorInterpolater;

protected:
	std::unique_ptr<CControlPointGridView<4>> m_colorControlPointGridView;
	std::unique_ptr<CInterpolaterSelectPlotter<4, true>> m_colorInterpolationSelectPlotter;

protected:
	void AdjustControlPointsFromLife();

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

public:
	virtual void DrawPropertyUI() override;
};

