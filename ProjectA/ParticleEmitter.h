#pragma once
#include "EmitterSpawnProperty.h"
#include "ParticleSpawnProperty.h"

struct SParticle
{
	DirectX::XMFLOAT3 position;
	float life;
	DirectX::XMFLOAT3 velocity;
	float mass;
	DirectX::XMFLOAT3 accelerate;
	UINT type;
};

struct SParticleSelector
{
	UINT index;
	UINT type;
	float zValue;
	UINT dummy;
};

class CParticleEmitter : public IUpdatable
{
public:
	CParticleEmitter(
		UINT emitterID,
		UINT emitterType,
		float particleDensity,
		bool& isEmitterWorldTransformChanged,
		DirectX::XMMATRIX& emitterWorldTransform,
		const DirectX::XMVECTOR& position,
		const DirectX::XMVECTOR& angle
	);

protected:
	DirectX::XMVECTOR m_position;
	DirectX::XMVECTOR m_angle;
	bool& m_isEmitterWorldTransformChanged;
	DirectX::XMMATRIX& m_emitterWorldTransform;
	bool m_isThisWorldTransformChanged;

protected:
	struct
	{
		DirectX::XMMATRIX emitterWorldTransform;
		UINT emitterID;
		UINT emitterType;
		float particleDenstiy;
		float dummy;
	} m_emitterPropertyCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_emitterPropertyGPU;
	bool m_isEmitterPropertyChanged = false;

public:
	inline UINT GetEmitterID() const noexcept { return m_emitterPropertyCPU.emitterID; }
	inline UINT GetEmitterType() const noexcept { return m_emitterPropertyCPU.emitterType; }
	inline float GetParticleDensity() const noexcept { return m_emitterPropertyCPU.particleDenstiy; }
	void SetParticleDensity(float particleDensity);

public:
	inline ID3D11Buffer* GetEmitterPropertyBuffer() const noexcept { return m_emitterPropertyGPU->GetBuffer(); }

protected:
	std::unique_ptr<CEmitterSpawnProperty> m_emitterSpawnProperty;
	std::unique_ptr<CParticleSpawnProperty> m_particleSpawnProperty;

public:
	inline CEmitterSpawnProperty* GetEmitterSpawnProperty() noexcept { return m_emitterSpawnProperty.get(); }
	inline CParticleSpawnProperty* GetParticleSpawnProperty() noexcept { return m_particleSpawnProperty.get(); }

public:
	void SetPosition(const DirectX::XMVECTOR& position) noexcept;
	void SetAngle(const DirectX::XMVECTOR& angle) noexcept;
	inline const DirectX::XMVECTOR& GetPosition() const noexcept { return m_position; }
	inline const DirectX::XMVECTOR& GetAngle() const noexcept { return m_angle; }
	

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;
};

