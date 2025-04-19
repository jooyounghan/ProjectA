#pragma once
#include "IProperty.h"
#include <functional>
#include "InterpolationSelector.h"

#define LoopInfinity static_cast<UINT8>(~0)

template<uint32_t Dim>
class IInterpolater;

template<uint32_t Dim>
class ControlPointGridView;

template<uint32_t Dim>
class InterpolationSelectPlotter;

enum class EInterpolationMethod;

typedef std::function<void(class BaseEmitterUpdateProperty*)> OnEmitterDispose;

class BaseEmitterUpdateProperty : public APropertyHasLoopTime
{
public:
	BaseEmitterUpdateProperty(float& emitterCurrentTime, float& loopTime);
	virtual ~BaseEmitterUpdateProperty() = default;

protected:
	float& m_emitterCurrentTime;

protected:
	UINT8 m_loopCount;
	SControlPoint<1> m_spawnInitControlPoint;
	SControlPoint<1> m_spawnFinalControlPoint;
	std::vector<SControlPoint<1>> m_spawnControlPoints;
	EInterpolationMethod m_spawnRateInterpolationMethod;
	std::unique_ptr<IInterpolater<1>> m_spawnRateInterpolater;

protected:
	std::unique_ptr<ControlPointGridView<1>> m_spawnRateControlPointGridView;
	std::unique_ptr<InterpolationSelectPlotter<1>> m_spawnRateInterpolaterSelectPlotter;

protected:
	virtual void AdjustControlPointsFromLoopTime() override;

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

