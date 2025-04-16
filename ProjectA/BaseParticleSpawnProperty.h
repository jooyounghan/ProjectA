#pragma once
#include "IProperty.h"
#include "DynamicBuffer.h"
#include "InterpolateHelper.h"

#include <d3d11.h>
#include <vector>

class BaseParticleSpawnProperty : public IProperty<BaseParticleSpawnProperty>
{
public:
	BaseParticleSpawnProperty(
		const float& emitterDeltaTimeRef,
		const DirectX::XMFLOAT2& minMaxLifeTime,
		const DirectX::XMFLOAT2& minEmitRadian,
		const DirectX::XMFLOAT2& maxEmitRadian,
		EInterpolationMethod speedInterpolationMethod = EInterpolationMethod::Linear,
		const std::vector<SControlPoint2>& speedControlPoints = std::vector<SControlPoint2>(),
		EInterpolationMethod colorInterpolationMethod = EInterpolationMethod::Linear,
		const std::vector<SControlPoint3>& colorControlPoints = std::vector<SControlPoint3>()
	);
	virtual ~BaseParticleSpawnProperty() = default;

protected:
	const float& m_emitterDeltaTimeRef;

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
	bool m_isParticleSpawnPropertyChanged = false;

protected:
	std::vector<SControlPoint2> m_speedControlPoints;
	EInterpolationMethod m_speedInterpolationMethod;
	FPoint2Interpolater m_speedInterpolater;

public:
	void SetSpeedControlPoints(const std::vector<SControlPoint2>& speedControlPoints);
	void SetSpeedInterpolationMethod(EInterpolationMethod spawnRateInterpolationMethod);

protected:
	std::vector<SControlPoint3> m_colorControlPoints;
	EInterpolationMethod m_colorInterpolationMethod;
	FPoint3Interpolater m_colorInterpolater;

public:
	void SetColorControlPoints(const std::vector<SControlPoint3>& colorControlPoints);
	void SetColorInterpolationMethod(EInterpolationMethod spawnRateInterpolationMethod);

public:
	void SetMinMaxLifeTime(const DirectX::XMFLOAT2& minMaxLifeTime);
	void SetMinEmitRadian(const DirectX::XMFLOAT2& minEmitRadian);
	void SetMaxEmitRadian(const DirectX::XMFLOAT2& maxEmitRadian);
	
public:
	inline const DirectX::XMFLOAT2& GetMinMaxLifeTime() const noexcept { return m_baseParticleSpawnPropertyCPU.minMaxLifeTime; }
	inline const DirectX::XMFLOAT2& GetMinEmitRadian() const noexcept { return m_baseParticleSpawnPropertyCPU.minEmitRadian; }
	inline const DirectX::XMFLOAT2& GetMaxEmitRadian() const noexcept { return m_baseParticleSpawnPropertyCPU.maxEmitRadian; }

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;
	virtual void DrawPropertyUI() override;

public:
	static std::unique_ptr<BaseParticleSpawnProperty> DrawPropertyCreator();
};

