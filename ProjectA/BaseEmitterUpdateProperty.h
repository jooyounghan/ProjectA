#pragma once
#include "IProperty.h"
#include "InterpolaterStructure.h"

#include <functional>

#define LoopInfinity static_cast<UINT8>(~0)

template<uint32_t Dim, bool GPUInterpolateOn>
class IInterpolater;

template<uint32_t Dim>
class ControlPointGridView;

template<uint32_t Dim, bool GPUInterpolateOn>
class InterpolationSelectPlotter;

typedef std::function<void(class BaseEmitterUpdateProperty*)> OnEmitterDispose;

class BaseEmitterUpdateProperty : public APropertyHasLoopTime
{
public:
	BaseEmitterUpdateProperty(float& emitterCurrentTime, float& loopTime);
	~BaseEmitterUpdateProperty() override = default;

protected:
	float& m_emitterCurrentTime;

protected:
	bool m_isLoopInfinity;

protected:
	UINT8 m_loopCount;
	SControlPoint<1> m_spawnInitControlPoint;
	SControlPoint<1> m_spawnFinalControlPoint;
	std::vector<SControlPoint<1>> m_spawnControlPoints;
	EInterpolationMethod m_spawnRateInterpolationMethod;
	std::unique_ptr<IInterpolater<1, false>> m_spawnRateInterpolater;

protected:
	std::unique_ptr<ControlPointGridView<1>> m_spawnRateControlPointGridView;
	std::unique_ptr<InterpolationSelectPlotter<1, false>> m_spawnRateInterpolaterSelectPlotter;

protected:
	virtual void AdjustControlPointsFromLoopTime() override;

protected:
	bool m_isNotDisposed;
	OnEmitterDispose m_emitterDisposeHandler = bind([&](BaseEmitterUpdateProperty*) {}, std::placeholders::_1);

public:
	inline void SetEmitterDisposeHandler(OnEmitterDispose emitterDisposeHandler) noexcept { m_emitterDisposeHandler = emitterDisposeHandler; }

protected:
	float m_spawnCount;
	UINT m_saturatedSpawnCount;

public:
	inline UINT GetSpawnCount() const noexcept { return m_saturatedSpawnCount; };

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

public:
	virtual void DrawPropertyUI() override;
};

