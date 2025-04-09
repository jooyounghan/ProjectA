#pragma once
#include "Updatable.h"
#include "DynamicBuffer.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>

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
		bool& isEmitterWorldTransformChanged,
		DirectX::XMMATRIX& emitterWorldTransform,
		const DirectX::XMVECTOR& position,
		const DirectX::XMVECTOR& angle,
		const DirectX::XMVECTOR& emitVelocity
	);

protected:
	DirectX::XMVECTOR m_position;
	DirectX::XMVECTOR m_angle;
	bool& m_isEmitterWorldTransformChanged;
	DirectX::XMMATRIX& m_emitterWorldTransform;
	bool m_isThisWorldTransformChanged;

public:
	void SetPosition(const DirectX::XMVECTOR& position) noexcept;
	void SetAngle(const DirectX::XMVECTOR& angle) noexcept;
	inline const DirectX::XMVECTOR& GetPosition() const noexcept { return m_position; }
	inline const DirectX::XMVECTOR& GetAngle() const noexcept { return m_angle; }

protected:
	struct  
	{
		DirectX::XMMATRIX emitterWorldTransform;
		DirectX::XMFLOAT3 emitVelocity;
		UINT emitterID;
		UINT emitterType;
		float particleDensity;
		UINT dummy[3];
	} m_emitterPropertiesCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_emitterPropertiesGPU;
	bool m_isEmitterPropertiesChanged;

public:
	inline ID3D11Buffer* GetEmitterPropertiesBuffer() const noexcept { return m_emitterPropertiesGPU->GetBuffer(); }

public:
	void SetEmitVelocity(const DirectX::XMVECTOR& emitVelocity) noexcept;
	void SetEmitterID(UINT emitterID) noexcept;
	inline const DirectX::XMFLOAT3& GetEmitVelocity() const noexcept { return m_emitterPropertiesCPU.emitVelocity; }
	inline const UINT& GetEmitterID() const noexcept { return m_emitterPropertiesCPU.emitterID; }

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;
};

