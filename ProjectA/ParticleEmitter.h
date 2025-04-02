#pragma once
#include "Updatable.h"
#include "ConstantBuffer.h"
#include "DynamicBuffer.h"

#include <DirectXMath.h>
#include <vector>

class CParticleEmitter : public IUpdatable
{
public:
	CParticleEmitter(
		const DirectX::XMVECTOR& position,
		const DirectX::XMVECTOR& angle,
		const DirectX::XMVECTOR& emitVelocity
	);

protected:
	static const std::vector<DirectX::XMFLOAT3> GEmitterBoxPositions;
	static const std::vector<UINT> GEmitterBoxIndices;

protected:
	DirectX::XMVECTOR m_position;
	DirectX::XMVECTOR m_angle;

// TODO : DrawInstanced
protected:
	D3D11::CConstantBuffer m_positionBuffer;
	D3D11::CConstantBuffer m_indexBuffer;

public:
	inline std::vector<ID3D11Buffer*> GetVertexBuffers() const noexcept { return { m_positionBuffer.GetBuffer() }; }
	inline ID3D11Buffer* GetIndexBuffer() const noexcept { return m_indexBuffer.GetBuffer(); }
	inline std::vector<UINT> GetStrides() const noexcept { return { sizeof(DirectX::XMFLOAT3) }; }
	inline std::vector<UINT> GetOffsets() const noexcept { return { 0 }; }
	inline UINT GetIndexCount() const noexcept { return static_cast<UINT>(CParticleEmitter::GEmitterBoxIndices.size()); }

protected:
	struct 
	{
		DirectX::XMMATRIX toWorldTransform;
		DirectX::XMVECTOR emitVelocity;
	} m_emitterPropertiesCPU;
	D3D11::CDynamicBuffer m_emitterPropertiesGPU;
	bool m_isEmitterPropertiesChanged;

public:
	inline ID3D11Buffer* GetPropertiesBuffer() const noexcept { return m_emitterPropertiesGPU.GetBuffer(); }

public:
	void SetPosition(const DirectX::XMVECTOR& position) noexcept;
	void SetAngle(const DirectX::XMVECTOR& angle) noexcept;
	void SetEmitVelocity(const DirectX::XMVECTOR& emitVelocity) noexcept;

public:
	inline const DirectX::XMVECTOR& GetPosition() const noexcept { return m_position; }
	inline const DirectX::XMVECTOR& GetAngle() const noexcept { return m_angle; }
	inline const DirectX::XMVECTOR& GetEmitVelocity() const noexcept { return m_emitterPropertiesCPU.emitVelocity; }

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;
};

