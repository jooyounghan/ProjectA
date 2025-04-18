#pragma once
#include "IProperty.h"
#include "DynamicBuffer.h"
#include "InterpolationSelector.h"
#include "ShapedVectorSelector.h"


class BaseParticleSpawnProperty : public IProperty
{
public:
	BaseParticleSpawnProperty(float& emitterCurrentTime);
	virtual ~BaseParticleSpawnProperty() = default;

protected:
	float& m_emitterCurrentTime;

public:
	inline void SetEmitterCurrentTime(float emitterCurrentTime) { m_emitterCurrentTime = emitterCurrentTime; }

protected:
	struct  
	{
		SShapedVectorProperty shapedSpeedVectorSelector;
		DirectX::XMFLOAT2 minMaxLifeTime;
		DirectX::XMFLOAT3 color;
		UINT dummy;
	} m_baseParticleSpawnPropertyCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_baseParticleSpawnPropertyGPU;

protected:
	DirectX::XMFLOAT3 m_origin;
	DirectX::XMVECTOR m_upVector;

protected:
	SControlPoint<2> m_lifeInitControlPoint;
	SControlPoint<2> m_lifeFinalControlPoint;
	std::vector<SControlPoint<2>> m_lifeControlPoints;
	EInterpolationMethod m_lifeInterpolationMethod;
	std::unique_ptr<IInterpolater<2>> m_lifeInterpolater;

protected:
	SControlPoint<3> m_colorInitControlPoint;
	SControlPoint<3> m_colorFinalControlPoint;
	std::vector<SControlPoint<3>> m_colorControlPoints;
	EInterpolationMethod m_colorInterpolationMethod;
	std::unique_ptr<IInterpolater<3>> m_colorInterpolater;

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

public:
	virtual void DrawPropertyUI() override;
};

