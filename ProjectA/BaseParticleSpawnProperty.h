#pragma once
#include "IProperty.h"
#include "DynamicBuffer.h"
#include "InterpolationSelector.h"

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
		DirectX::XMFLOAT2 minMaxLifeTime;
		DirectX::XMFLOAT2 minEmitRadian;
		DirectX::XMFLOAT2 maxEmitRadian;
		DirectX::XMFLOAT2 minMaxSpeed;
		DirectX::XMFLOAT3 color;
		UINT dummy;
	} m_baseParticleSpawnPropertyCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_baseParticleSpawnPropertyGPU;

public:
	void SetMinMaxLifeTime(const DirectX::XMFLOAT2& minMaxLifeTime);
	void SetMinEmitRadian(const DirectX::XMFLOAT2& minEmitRadian);
	void SetMaxEmitRadian(const DirectX::XMFLOAT2& maxEmitRadian);

public:
	inline const DirectX::XMFLOAT2& GetMinMaxLifeTime() const noexcept { return m_baseParticleSpawnPropertyCPU.minMaxLifeTime; }
	inline const DirectX::XMFLOAT2& GetMinEmitRadian() const noexcept { return m_baseParticleSpawnPropertyCPU.minEmitRadian; }
	inline const DirectX::XMFLOAT2& GetMaxEmitRadian() const noexcept { return m_baseParticleSpawnPropertyCPU.maxEmitRadian; }

protected:
	SControlPoint<2> m_speedInitControlPoint;
	SControlPoint<2> m_speedFinalControlPoint;
	std::vector<SControlPoint<2>> m_speedControlPoints;
	EInterpolationMethod m_speedInterpolationMethod;
	std::unique_ptr<IInterpolater<2>> m_speedInterpolater;

public:
	void SetInitSpeed(const SControlPoint<2>& speed) noexcept;
	void SetFinalSpeed(const SControlPoint<2>& speed) noexcept;
	void SetSpeedControlPoints(const std::vector<SControlPoint<2>>& speedControlPoints);
	void SetSpeedInterpolationMethod(EInterpolationMethod speedInterpolationMethod);

protected:
	SControlPoint<3> m_colorInitControlPoint;
	SControlPoint<3> m_colorFinalControlPoint;
	std::vector<SControlPoint<3>> m_colorControlPoints;
	EInterpolationMethod m_colorInterpolationMethod;
	std::unique_ptr<IInterpolater<3>> m_colorInterpolater;

public:
	void SetInitColor(const SControlPoint<3>& color) noexcept;
	void SetFinalColor(const SControlPoint<3>& color) noexcept;
	void SetColorControlPoints(const std::vector<SControlPoint<3>>& colorControlPoints);
	void SetColorInterpolationMethod(EInterpolationMethod colorInterpolationMethod);

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

public:
	virtual void DrawPropertyUI() override;
};

