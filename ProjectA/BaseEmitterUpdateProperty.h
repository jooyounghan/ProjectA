#pragma once
#include "IProperty.h"
#include "InterpolationSelector.h"

#include <functional>

#define LoopInfinity static_cast<UINT8>(~0)


typedef std::function<void(class BaseEmitterUpdateProperty*)> OnEmitterDispose;

class BaseEmitterUpdateProperty : public IProperty
{
public:
	BaseEmitterUpdateProperty(float& emitterCurrentTime);
	virtual ~BaseEmitterUpdateProperty() = default;

protected:
	float& m_emitterCurrentTime;

protected:
	UINT8 m_loopCount;
	float m_loopTime;
	SControlPoint m_spawnInitControlPoint;
	SControlPoint m_spawnFinalControlPoint;
	std::vector<SControlPoint> m_spawnControlPoints;
	EInterpolationMethod m_spawnRateInterpolationMethod;
	std::unique_ptr<IInterpolater> m_spawnRateInterpolater;

//public:
//	inline UINT8 GetLoopCount() const noexcept { return m_loopCount; }
//	inline float GetLoopTime() const noexcept { return m_loopTime; }
//	inline const std::vector<SControlPoint>& GetSpawnControlPoints() const noexcept { return m_spawnControlPoints; }
//	inline EInterpolationMethod GetSpawnRateInterpolationMethod() const noexcept { return m_spawnRateInterpolationMethod; }

public:
	inline void SetLoopCount(UINT loopCount) noexcept { m_loopCount = loopCount; }
	void SetLoopTime( float loopTime) noexcept;
	inline void SetInitSpawnRate(float spawnRate) noexcept { m_spawnInitControlPoint.y = spawnRate; }
	inline void SetFinalSpawnRate(float spawnRate) noexcept { m_spawnFinalControlPoint.y = spawnRate; }
	void SetSpawnControlPoints(const std::vector<SControlPoint>& spawnControlPoints) noexcept;
	void SetSpawnRateInterpolationMethod(EInterpolationMethod spawnRateInterpolationMethod);

protected:
	bool m_isNotDisposed;
	OnEmitterDispose m_emitterDisposeHandler = bind([&](BaseEmitterUpdateProperty*) {}, std::placeholders::_1);

public:
	inline void SetEmitterDisposeHandler(OnEmitterDispose emitterDisposeHandler) noexcept { m_emitterDisposeHandler = emitterDisposeHandler; }

public:
	float GetSpawnRate() const;

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

public:
	virtual void DrawPropertyUI() override;
};

