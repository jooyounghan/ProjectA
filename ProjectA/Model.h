#pragma once
#include "Updatable.h"
#include "ConstantBuffer.h"
#include "DynamicBuffer.h"

#include <DirectXMath.h>
#include <vector>
#include <memory>


class CModel : public IUpdatable
{
public:
	CModel
	(
		const DirectX::XMVECTOR& position,
		const DirectX::XMVECTOR& angle,
		const DirectX::XMVECTOR& scale
	);
	virtual ~CModel() = 0;

protected:
	DirectX::XMVECTOR m_position;
	DirectX::XMVECTOR m_angle;
	DirectX::XMVECTOR m_scale;

protected:
	std::unique_ptr<D3D11::CConstantBuffer> m_positionBuffer;
	std::unique_ptr<D3D11::CConstantBuffer> m_uvCoordBuffer;
	std::unique_ptr<D3D11::CConstantBuffer> m_normalBuffer;
	std::unique_ptr<D3D11::CConstantBuffer> m_indexBuffer;

public:
	void SetPositionBuffer(const std::vector<DirectX::XMFLOAT3>& positions);
	void SetUVCoordBuffer(const std::vector<DirectX::XMFLOAT2>& uvCoords);
	void SetNormalBuffer(const std::vector<DirectX::XMFLOAT3>& normals);
	void SetIndexBuffer(const std::vector<UINT>& indices);

public:
	std::vector<ID3D11Buffer*> GetVertexBuffers() const noexcept;
	ID3D11Buffer* GetIndexBuffer() const noexcept;

protected:
	struct 
	{
		DirectX::XMMATRIX toWorldTransform;
		DirectX::XMMATRIX toWorldTransformInv;
		DirectX::XMMATRIX toWorldTransformInvT;
	} m_worldTransformationCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_worldTransformationGPU;
	bool m_isWorldTransformationChanged;

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;
};

