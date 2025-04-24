#pragma once
#include "Updatable.h"
#include "EmitterForceProperty.h"
#include "EmitterStaticData.h"

class CBaseEmitterSpawnProperty;
class CBaseEmitterUpdateProperty;
class CBaseParticleSpawnProperty;
class CBaseParticleUpdateProperty;

namespace D3D11
{
	class CDynamicBuffer;
}

struct SParticle
{
	DirectX::XMVECTOR color;
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 velocity;
	DirectX::XMFLOAT3 accelerate;
	UINT emitterType;
	UINT emitterID;
	float life;
};

class AEmitter : public IUpdatable
{
public:
	AEmitter(
		UINT emitterType,
		UINT emitterID,
		const DirectX::XMVECTOR& position,
		const DirectX::XMVECTOR& angle
	);

protected:
	struct alignas(16)
	{
		DirectX::XMMATRIX emitterWorldTransform;
		UINT emitterType;
		UINT emitterID;
		DirectX::XMFLOAT2 padding;
	} m_emitterPropertyCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_emitterPropertyGPU;
	bool m_isEmitterPropertyChanged;

public:
	inline UINT GetEmitterType() const noexcept { return m_emitterPropertyCPU.emitterType; }
	inline UINT GetEmitterID() const noexcept { return m_emitterPropertyCPU.emitterID; }

public:
	ID3D11Buffer* GetEmitterPropertyBuffer() const noexcept;

protected:
	bool m_isSpawned;

public:
	inline bool IsSpawned() const noexcept { return m_isSpawned; }
	inline void SetSpawned(bool isSpawned) noexcept { m_isSpawned = isSpawned; }

protected:
	float m_emitterCurrentTime;
	float m_emitterLoopTime;

public:
	inline float& GetEmitterCurrentTimeRef() noexcept { return m_emitterCurrentTime; }
	inline float& GetEmitterLoopTimeRef() noexcept { return m_emitterLoopTime; }

protected:
	DirectX::XMVECTOR m_position;
	DirectX::XMVECTOR m_angle;

public:
	void SetEmitterForceProperty(const SEmitterForceProperty& emitterForce);
	void SetInterpolaterLifeInformation(float life);
	void SetColorInterpolaterInformation(UINT interpolaterID, UINT interpolaterDegree);

protected:
	std::unique_ptr<CBaseEmitterSpawnProperty> m_emitterSpawnProperty;
	std::unique_ptr<CBaseEmitterUpdateProperty> m_emitterUpdateProperty;
	std::unique_ptr<CBaseParticleSpawnProperty> m_particleSpawnProperty;
	std::unique_ptr<CBaseParticleUpdateProperty> m_particleUpdateProperty;

public:
	void InjectAEmitterSpawnProperty(std::unique_ptr<CBaseEmitterSpawnProperty>& emitterSpawnProperty) noexcept;
	void InjectAEmitterUpdateProperty(std::unique_ptr<CBaseEmitterUpdateProperty>& emitterUpdateProperty) noexcept;
	void InjectAParticleSpawnProperty(std::unique_ptr<CBaseParticleSpawnProperty>& particleSpawnProperty) noexcept;
	void InjectAParticleUpdateProperty(std::unique_ptr<CBaseParticleUpdateProperty>& particleSpawnProperty) noexcept;

public:
	inline CBaseEmitterSpawnProperty* GetAEmitterSpawnProperty() const noexcept { return m_emitterSpawnProperty.get(); }
	inline CBaseEmitterUpdateProperty* GetAEmitterUpdateProperty() const noexcept { return m_emitterUpdateProperty.get(); }
	inline CBaseParticleSpawnProperty* GetAParticleSpawnProperty() const noexcept { return m_particleSpawnProperty.get(); }
	inline CBaseParticleUpdateProperty* GetAParticleUpdateProperty() const noexcept { return m_particleUpdateProperty.get(); }

public:
	void SetPosition(const DirectX::XMVECTOR& position) noexcept;
	void SetAngle(const DirectX::XMVECTOR& angle) noexcept;
	inline const DirectX::XMVECTOR& GetPosition() const noexcept { return m_position; }
	inline const DirectX::XMVECTOR& GetAngle() const noexcept { return m_angle; }

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;
};


