#pragma once
#include "Updatable.h"
#include <Windows.h>
#include <DirectXMath.h>
#include <memory>

class BaseEmitterSpawnProperty;
class BaseEmitterUpdateProperty;
class BaseParticleSpawnProperty;
class BaseParticleUpdateProperty;

struct SParticle
{
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
		bool& isEmitterWorldTransformChanged,
		DirectX::XMMATRIX& emitterWorldTransformRef,
		const DirectX::XMVECTOR& position,
		const DirectX::XMVECTOR& angle
	);

protected:
	DirectX::XMVECTOR m_position;
	DirectX::XMVECTOR m_angle;

protected:
	bool& m_isEmitterWorldTransformChangedRef;
	DirectX::XMMATRIX& m_emitterWorldTransformRef;
	bool m_isThisWorldTransformChanged;

protected:
	std::unique_ptr<BaseEmitterSpawnProperty> m_emitterSpawnProperty;
	std::unique_ptr<BaseEmitterUpdateProperty> m_emitterUpdateProperty;
	std::unique_ptr<BaseParticleSpawnProperty> m_particleSpawnProperty;
	std::unique_ptr<BaseParticleUpdateProperty> m_particleUpdateProperty;

public:
	inline void InjectAEmitterSpawnProperty(std::unique_ptr<BaseEmitterSpawnProperty> emitterSpawnProperty) noexcept { m_emitterSpawnProperty = std::move(emitterSpawnProperty); }
	inline void InjectAEmitterUpdateProperty(std::unique_ptr<BaseEmitterUpdateProperty> emitterUpdateProperty) noexcept { m_emitterUpdateProperty = std::move(emitterUpdateProperty); }
	inline void InjectAParticleSpawnProperty(std::unique_ptr<BaseParticleSpawnProperty> particleSpawnProperty) noexcept { m_particleSpawnProperty = std::move(particleSpawnProperty); }
	inline void InjectAParticleUpdateProperty(std::unique_ptr<BaseParticleUpdateProperty> particleSpawnProperty) noexcept { m_particleUpdateProperty = std::move(particleSpawnProperty); }

public:
	inline BaseEmitterSpawnProperty* GetAEmitterSpawnProperty() const noexcept { return m_emitterSpawnProperty.get(); }
	inline BaseEmitterUpdateProperty* GetAEmitterUpdateProperty() const noexcept { return m_emitterUpdateProperty.get(); }
	inline BaseParticleSpawnProperty* GetAParticleSpawnProperty() const noexcept { return m_particleSpawnProperty.get(); }
	inline BaseParticleUpdateProperty* GetAParticleUpdateProperty() const noexcept { return m_particleUpdateProperty.get(); }

public:
	void SetPosition(const DirectX::XMVECTOR& position) noexcept;
	void SetAngle(const DirectX::XMVECTOR& angle) noexcept;
	inline const DirectX::XMVECTOR& GetPosition() const noexcept { return m_position; }
	inline const DirectX::XMVECTOR& GetAngle() const noexcept { return m_angle; }

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;
};


