#pragma once
#include "IProperty.h"
#include "InterpolationSelector.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>


#define LoopInfinity static_cast<UINT8>(~0)


typedef std::function<void(class BaseEmitterUpdateProperty*)> OnEmitterDispose;

class BaseEmitterUpdateProperty : public IProperty
{
public:
	BaseEmitterUpdateProperty(
		UINT8 loopCount,
		float loopTime,
		const std::vector<SControlPoint>& spawnControlPoints,
		EInterpolationMethod spawnRateInterpolationMethod
	);
	virtual ~BaseEmitterUpdateProperty() = default;

protected:
	float* m_emitterCurrentTime = nullptr;

public:
	inline void SetEmitterCurrentTime(const float* emitterCurrentTime) { m_emitterCurrentTime = m_emitterCurrentTime; }


protected:
	UINT8 m_loopCount;
	float m_loopTime;
	std::vector<SControlPoint> m_spawnControlPoints;
	EInterpolationMethod m_spawnRateInterpolationMethod;
	std::unique_ptr<AInterpolater> m_spawnRateInterpolater;

public:
	void SetSpawnControlPoints(const std::vector<SControlPoint>& spawnControlPoints) noexcept;
	void SetSpawnRateInterpolationMethod(EInterpolationMethod spawnRateInterpolationMethod);

public:
	inline UINT GetLoopCount() const noexcept { return m_loopCount; }
	inline const std::vector<SControlPoint>& GetSpawnControlPoints() const noexcept { return m_spawnControlPoints; }
	inline EInterpolationMethod GetSpawnRateInterpolationMethod() const noexcept { return m_spawnRateInterpolationMethod; }

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

public:
	static std::unique_ptr<BaseEmitterUpdateProperty> DrawPropertyCreator(bool& isApplied, float& emitterCurrentTimeRef);
};

