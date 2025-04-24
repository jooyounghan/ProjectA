#pragma once
#include "IProperty.h"
#include "InterpolaterStructure.h"

#include <functional>
#include <memory>

#define LoopInfinity static_cast<UINT8>(~0)

template<uint32_t Dim>
class IInterpolater;

template<uint32_t Dim>
class CControlPointGridView;

template<uint32_t Dim>
class CInterpolaterSelectPlotter;

typedef std::function<void(class CBaseEmitterUpdateProperty*)> OnEmitterDispose;

class CBaseEmitterUpdateProperty : public APropertyOnEmitterTimeline
{
public:
	CBaseEmitterUpdateProperty(float& emitterCurrentTime, float& emitterLoopTime);
	~CBaseEmitterUpdateProperty() override = default;

protected:
	bool m_isLoopInfinity;

protected:
	UINT8 m_loopCount;
	SControlPoint<1> m_spawnInitControlPoint;
	SControlPoint<1> m_spawnFinalControlPoint;
	std::vector<SControlPoint<1>> m_spawnControlPoints;
	EInterpolationMethod m_spawnRateInterpolationMethod;
	std::unique_ptr<IInterpolater<1>> m_spawnRateInterpolater;

protected:
	std::unique_ptr<CControlPointGridView<1>> m_spawnRateControlPointGridView;
	std::unique_ptr<CInterpolaterSelectPlotter<1>> m_spawnRateInterpolaterSelectPlotter;

protected:
	virtual void AdjustControlPointsFromLoopTime() override;

protected:
	bool m_isNotDisposed;
	OnEmitterDispose m_emitterDisposeHandler = bind([&](CBaseEmitterUpdateProperty*) {}, std::placeholders::_1);

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

