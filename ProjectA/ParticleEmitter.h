#pragma once
#include "EmitterSpawnProperty.h"
#include "ParticleSpawnProperty.h"

struct SParticle
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 velocity;
	DirectX::XMFLOAT3 accelerate;
	float life;
	float density;
	UINT emitterType;
	UINT emitterID;
	float radius;
	float dummy[2];
};

struct SParticleSelector
{
	DirectX::XMVECTOR viewPos;
	UINT index;
	UINT emitterType;
	float depth;
	float dummy;
};

class CParticleEmitter : public IUpdatable
{
public:
	CParticleEmitter(
		UINT emitterID,
		UINT emitterType,
		float particleDensity,
		float particleRadius,
		bool& isEmitterWorldPositionChanged,
		bool& isEmitterWorldTransformChanged,
		DirectX::XMVECTOR& positionRef,
		DirectX::XMMATRIX& emitterWorldTransformRef,
		const DirectX::XMVECTOR& position,
		const DirectX::XMVECTOR& angle,
		const DirectX::XMFLOAT2& minInitRadians,
		const DirectX::XMFLOAT2& maxInitRadians,
		const DirectX::XMFLOAT2& minMaxRadius,
		UINT initialParticleCount
	);

protected:
	DirectX::XMVECTOR m_angle;

protected:
	bool& m_isEmitterWorldPositionChanged;
	bool& m_isEmitterWorldTransformChanged;
	DirectX::XMVECTOR& m_positionRef;
	DirectX::XMMATRIX& emitterWorldTransformRef;
	bool m_isThisWorldTransformChanged;

protected:
	struct
	{
		DirectX::XMMATRIX emitterWorldTransform;
		UINT emitterID;
		UINT emitterType;
		float particleDenstiy;
		float particleRadius;
	} m_emitterPropertyCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_emitterPropertyGPU;
	bool m_isEmitterPropertyChanged = false;

public:
	inline UINT GetEmitterID() const noexcept { return m_emitterPropertyCPU.emitterID; }
	inline UINT GetEmitterType() const noexcept { return m_emitterPropertyCPU.emitterType; }
	inline float GetParticleDensity() const noexcept { return m_emitterPropertyCPU.particleDenstiy; }
	inline float GetParticleRadius() const noexcept { return m_emitterPropertyCPU.particleRadius; }
	void SetParticleDensity(float particleDensity);
	void SetParticleRadius(float particleRadius);

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
	inline const DirectX::XMVECTOR& GetPosition() const noexcept { return m_positionRef; }
	inline const DirectX::XMVECTOR& GetAngle() const noexcept { return m_angle; }
	

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;
};

