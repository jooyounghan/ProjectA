#pragma once
#include "Updatable.h"
#include "BaseEmitterSpawnProperty.h"
#include "BaseEmitterUpdateProperty.h"
#include "BaseParticleSpawnProperty.h"
#include "BaseParticleUpdateProperty.h"
#include "StructuredBuffer.h"

#include <DirectXMath.h>
#include <memory>
#include <queue>


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
protected:
	static UINT GEmitterMaxCount;
	static std::queue<UINT> GEmitterIDQueue;

public:
	static std::vector<DirectX::XMMATRIX> GEmitterWorldTransformCPU;
	static std::unique_ptr<D3D11::CDynamicBuffer> GEmitterWorldTransformGPU;
	static std::vector<SEmitterForceProperty> GEmitterForcePropertyCPU;
	static std::unique_ptr<D3D11::CStructuredBuffer> GEmitterForcePropertyGPU;
	static bool GIsEmitterWorldPositionChanged;
	static bool GIsEmitterForceChanged;

public:
	static void InitializeGlobalEmitterProperty(UINT emitterMaxCount);
	static void UpdateGlobalEmitterProperty(ID3D11DeviceContext* deviceContext);
	static UINT IssueAvailableEmitterID();

public:
	AEmitter(
		UINT emitterType,
		UINT emitterID,
		bool& isEmitterWorldTransformChanged,
		DirectX::XMMATRIX& emitterWorldTransform,
		bool& isEmitterForceChanged,
		SEmitterForceProperty& emitterForce,
		const DirectX::XMVECTOR& position,
		const DirectX::XMVECTOR& angle
	);

protected:
	DirectX::XMVECTOR m_position;
	DirectX::XMVECTOR m_angle;

protected:
	bool& m_isEmitterWorldTransformChanged;
	DirectX::XMMATRIX& m_emitterWorldTransform;
	bool m_isThisWorldTransformChanged;

protected:
	bool& m_isEmitterForceChanged;
	SEmitterForceProperty& m_emitterForce;

protected:
	float m_currnetEmitter = 0.f;

public:
	bool& GetIsEmitterForceChanged() { return m_isEmitterForceChanged; }
	SEmitterForceProperty& GetEmitterForce() { return m_emitterForce; }
	float& GetCurrnetEmitter() { return m_currnetEmitter; }

protected:
	std::unique_ptr<BaseEmitterSpawnProperty> m_emitterSpawnProperty;
	std::unique_ptr<BaseEmitterUpdateProperty> m_emitterUpdateProperty;
	std::unique_ptr<BaseParticleSpawnProperty> m_particleSpawnProperty;
	std::unique_ptr<BaseParticleUpdateProperty> m_particleUpdateProperty;

public:
	void InjectAEmitterSpawnProperty(std::unique_ptr<BaseEmitterSpawnProperty>& emitterSpawnProperty) noexcept;
	void InjectAEmitterUpdateProperty(std::unique_ptr<BaseEmitterUpdateProperty>& emitterUpdateProperty) noexcept;
	void InjectAParticleSpawnProperty(std::unique_ptr<BaseParticleSpawnProperty>& particleSpawnProperty) noexcept;
	void InjectAParticleUpdateProperty(std::unique_ptr<BaseParticleUpdateProperty>& particleSpawnProperty) noexcept;

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


