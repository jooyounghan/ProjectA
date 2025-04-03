#pragma once
#include "Updatable.h"
#include "DynamicBuffer.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>


class CParticleEmitter : public IUpdatable
{
public:
	struct SParticle
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 velocity;
		DirectX::XMFLOAT3 accelerate;
		float life;
		float mass;

	};

	struct SParticleSelector
	{
		uint32_t index24type8;
		float zValue;
	};

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

protected:
	struct  
	{
		DirectX::XMVECTOR emitVelocity;
		UINT emitterID;
		DirectX::XMFLOAT3 dummy;
	} m_emitterPropertiesCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_emittorPropertiesGPU;
	bool m_isEmitterPropertiesChanged;

public:
	void SetEmitVelocity(const DirectX::XMVECTOR& emitVelocity) noexcept;

public:
	inline const DirectX::XMVECTOR& GetPosition() const noexcept { return m_position; }
	inline const DirectX::XMVECTOR& GetAngle() const noexcept { return m_angle; }

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;
};

