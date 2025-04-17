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
	bool m_isParticleSpawnPropertyChanged = false;

protected:
	SControlPoint m_speedXInitControlPoint;
	SControlPoint m_speedYInitControlPoint;
	SControlPoint m_speedXFinalControlPoint;
	SControlPoint m_speedYFinalControlPoint;
	std::vector<SControlPoint> m_speedXControlPoints;
	std::vector<SControlPoint> m_speedYControlPoints;
	EInterpolationMethod m_speedInterpolationMethod;
	std::unique_ptr<IInterpolater> m_speedXInterpolater;
	std::unique_ptr<IInterpolater> m_speedYInterpolater;
	bool m_isSpeedInterpolaterChanged = false;

public:
	void SetSpeedControlPoints(
		const std::vector<SControlPoint>& speedXControlPoints,
		const std::vector<SControlPoint>& speedYControlPoints
	);
	void SetSpeedInterpolationMethod(EInterpolationMethod spawnRateInterpolationMethod);

protected:
	std::vector<SControlPoint> m_colorRControlPoints;
	std::vector<SControlPoint> m_colorGControlPoints;
	std::vector<SControlPoint> m_colorBControlPoints;
	EInterpolationMethod m_colorInterpolationMethod;
	std::unique_ptr<IInterpolater> m_colorRInterpolater;
	std::unique_ptr<IInterpolater> m_colorGInterpolater;
	std::unique_ptr<IInterpolater> m_colorBInterpolater;
	bool m_isColorInterpolaterChanged = false;

public:
	void SetColorControlPoints(
		const std::vector<SControlPoint>& colorRControlPoints,
		const std::vector<SControlPoint>& colorGControlPoints,
		const std::vector<SControlPoint>& colorBControlPoints
	);
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

public:
	virtual void DrawPropertyUI() override;
};

