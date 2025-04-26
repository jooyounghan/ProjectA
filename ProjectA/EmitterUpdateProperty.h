#pragma once
#include "IProperty.h"
#include "ISerializable.h"
#include "InterpolaterStructure.h"

#include <functional>
#include <memory>

template<uint32_t Dim>
class IInterpolater;

template<uint32_t Dim>
class CControlPointGridView;

template<uint32_t Dim>
class CInterpolaterSelectPlotter;

typedef std::function<void(class CEmitterUpdateProperty*)> OnEmitterDispose;

class CEmitterUpdateProperty : public IProperty, public ISerializable
{
public:
	CEmitterUpdateProperty();
	~CEmitterUpdateProperty() override = default;

protected:
	float m_currentTime;
	float m_loopTime;
	UINT8 m_currentLoopCount;
	UINT8 m_loopCount;

protected:
	void SetLoopCount(UINT8 loopCount);

protected:
	bool m_isLoopInfinity;

protected:
	SControlPoint<1> m_spawnInitControlPoint;
	SControlPoint<1> m_spawnFinalControlPoint;
	std::vector<SControlPoint<1>> m_spawnControlPoints;
	EInterpolationMethod m_spawnRateInterpolationMethod;
	std::unique_ptr<IInterpolater<1>> m_spawnRateInterpolater;

protected:
	std::unique_ptr<CControlPointGridView<1>> m_spawnRateControlPointGridView;
	std::unique_ptr<CInterpolaterSelectPlotter<1>> m_spawnRateInterpolaterSelectPlotter;

protected:
	void AdjustControlPointsFromLoopTime();

protected:
	bool m_isNotDisposed;
	OnEmitterDispose m_emitterDisposeHandler = bind([&](CEmitterUpdateProperty*) {}, std::placeholders::_1);

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

protected:
	virtual void DrawPropertyUIImpl() override;

public:
	virtual void Serialize(std::ofstream& ofs) override;
	virtual void Deserialize(std::ifstream& ifs) override;
};

