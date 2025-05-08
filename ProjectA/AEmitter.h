#pragma once
#include "IDrawable.h"
#include "IUpdatable.h"
#include "ISerializable.h"
#include "EmitterForceProperty.h"
#include "InterpolaterStructure.h"
#include "Interpolater.h"
#include "ParticleStructure.h"
#include "DynamicBuffer.h"

#include <functional>

class CInitialSpawnProperty;
class CEmitterUpdateProperty;
class CRuntimeSpawnProperty;
class ForceUpdateProperty;

class AEmitter : public IDrawable, public IUpdatable, public ISerializable
{
public:
	AEmitter(
		UINT emitterType,
		UINT emitterID,
		const DirectX::XMVECTOR& position,
		const DirectX::XMVECTOR& angle,
		const std::function<void(UINT, const DirectX::XMMATRIX&)>& worldTransformChangedHandler,
		const std::function<void(UINT, const SEmitterForceProperty&)>& forcePropertyChangedHandler,
		const std::function<void(UINT, UINT, bool, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterSelectedHandler,
		const std::function<void(UINT, UINT, bool, float, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterUpdatedHandler
	);
	~AEmitter() override = default;

protected:
	UINT m_emitterType;

protected:
	struct alignas(16)
	{
		const UINT emitterID;
		const UINT padding1;
		const UINT padding2;
		const UINT padding3;
	} m_emitterPropertyCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_emitterPropertyGPU;

public:
	inline UINT GetEmitterType() const noexcept { return m_emitterType; }
	inline UINT GetEmitterID() const noexcept { return m_emitterPropertyCPU.emitterID; }

public:
	inline ID3D11Buffer* GetEmitterPropertyBuffer() const noexcept { return m_emitterPropertyGPU->GetBuffer(); }

protected:
	bool m_isSpawned;

public:
	inline bool IsSpawned() const noexcept { return m_isSpawned; }
	inline void SetSpawned(bool isSpawned) noexcept { m_isSpawned = isSpawned; }

protected:
	DirectX::XMVECTOR m_position;
	DirectX::XMVECTOR m_angle;
	bool m_isEmitterWorldTransformChanged;

protected:
	UINT m_colorInterpolaterID;

public:
	inline UINT GetColorInterpolaterID() const noexcept { return m_colorInterpolaterID; }
	inline void SetColorInterpolaterID(UINT colorInterpolaterID) noexcept { m_colorInterpolaterID = colorInterpolaterID; }

protected:
	std::function<void(UINT, const DirectX::XMMATRIX&)> m_onWorldTransformChanged;
	std::function<void(UINT, const SEmitterForceProperty&)> m_onForcePropertyChanged;
	std::function<void(UINT, UINT, bool, EInterpolationMethod, IInterpolater<4>*)> m_onGpuColorInterpolaterSelected;
	std::function<void(UINT, UINT, bool, float, EInterpolationMethod, IInterpolater<4>*)> m_onGpuColorInterpolaterUpdated;

public:
	virtual void CreateProperty() = 0;

protected:
	std::unique_ptr<CInitialSpawnProperty> m_initialSpawnProperty;
	std::unique_ptr<CEmitterUpdateProperty> m_emitterUpdateProperty;
	std::unique_ptr<CRuntimeSpawnProperty> m_runtimeSpawnProperty;
	std::unique_ptr<ForceUpdateProperty> m_forceUpdateProperty;

public:
	inline CInitialSpawnProperty* GetInitialSpawnProperty() const noexcept { return m_initialSpawnProperty.get(); }
	inline CEmitterUpdateProperty* GetEmitterUpdateProperty() const noexcept { return m_emitterUpdateProperty.get(); }
	inline CRuntimeSpawnProperty* GetRuntimeSpawnProperty() const noexcept { return m_runtimeSpawnProperty.get(); }
	inline ForceUpdateProperty* GetForceUpdateProperty() const noexcept { return m_forceUpdateProperty.get(); }

public:
	void SetPosition(const DirectX::XMVECTOR& position) noexcept;
	void SetAngle(const DirectX::XMVECTOR& angle) noexcept;
	inline const DirectX::XMVECTOR& GetPosition() const noexcept { return m_position; }
	inline const DirectX::XMVECTOR& GetAngle() const noexcept { return m_angle; }

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

public:
	virtual void Serialize(std::ofstream& ofs) override;
	virtual void Deserialize(std::ifstream& ifs) override;

public:
	virtual void DrawUI() override;

protected:
	virtual void DrawUIImpl() override;
};


