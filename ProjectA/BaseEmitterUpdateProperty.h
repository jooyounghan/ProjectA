#pragma once
#include "IProperty.h"
#include "InterpolateHelper.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <vector>


#define LoopInfinity static_cast<uint8_t>(~0)


typedef std::function<void(class BaseEmitterUpdateProperty*)> OnEmitterDispose;

class BaseEmitterUpdateProperty : public IProperty<BaseEmitterUpdateProperty>
{
public:
	BaseEmitterUpdateProperty(
		float& emitterCurrentTimeRef,
		UINT8 loopCount,
		float loopTime,
		const std::vector<SControlPoint1>& spawnControlPoints = std::vector<SControlPoint1>(),
		EInterpolationMethod spawnRateInterpolationMethod = EInterpolationMethod::Linear
	);
	virtual ~BaseEmitterUpdateProperty() = default;

protected:
	float& m_emitterCurrentTimeRef;

protected:
	UINT m_loopCount;
	float m_loopTime;
	std::vector<SControlPoint1> m_spawnControlPoints;
	EInterpolationMethod m_spawnRateInterpolationMethod;
	FPoint1Interpolater m_spawnRateInterpolater;

public:
	void SetSpawnControlPoints(const std::vector<SControlPoint1>& spawnControlPoints) noexcept;
	void SetSpawnRateInterpolationMethod(EInterpolationMethod spawnRateInterpolationMethod);

public:
	inline UINT GetLoopCount() const noexcept { return m_loopCount; }
	inline const std::vector<SControlPoint1>& GetSpawnControlPoints() const noexcept { return m_spawnControlPoints; }
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
	virtual void DrawPropertyUI() override;

public:
	static std::unique_ptr<BaseEmitterUpdateProperty> DrawPropertyCreator();
};

