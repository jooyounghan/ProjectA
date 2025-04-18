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
	SControlPoint<1> m_spawnInitControlPoint;
	SControlPoint<1> m_spawnFinalControlPoint;
	std::vector<SControlPoint<1>> m_spawnControlPoints;
	EInterpolationMethod m_spawnRateInterpolationMethod;
	std::unique_ptr<IInterpolater<1>> m_spawnRateInterpolater;

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

