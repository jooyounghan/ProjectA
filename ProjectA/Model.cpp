#include "Model.h"
#include "BufferMacroUtilities.h"

using namespace std;
using namespace DirectX;
using namespace D3D11;

CModel::CModel(
	const XMVECTOR& position,
	const XMVECTOR& angle,
	const XMVECTOR& scale
)
	: m_position(position), m_angle(angle), m_scale(scale),
	m_isWorldTransformationChanged(false)
{

}

void CModel::SetPositionBuffer(const std::vector<DirectX::XMFLOAT3>& positions)
{
	m_positionBuffer.reset();
	m_positionBuffer = make_unique<CConstantBuffer>(
		12,
		static_cast<UINT>(positions.size()),
		positions.data(),
		D3D11_BIND_VERTEX_BUFFER
		);
}

void CModel::SetUVCoordBuffer(const std::vector<DirectX::XMFLOAT2>& uvCoords)
{
	m_uvCoordBuffer.reset();
	m_uvCoordBuffer = make_unique<CConstantBuffer>(
		8,
		static_cast<UINT>(uvCoords.size()),
		uvCoords.data(),
		D3D11_BIND_VERTEX_BUFFER
		);
}

void CModel::SetNormalBuffer(const std::vector<DirectX::XMFLOAT3>& normals)
{
	m_normalBuffer.reset();
	m_normalBuffer = make_unique<CConstantBuffer>(
		12,
		static_cast<UINT>(normals.size()),
		normals.data(),
		D3D11_BIND_VERTEX_BUFFER
		);
}

void CModel::SetIndexBuffer(const std::vector<UINT>& indices)
{
	m_indexBuffer.reset();
	m_indexBuffer = make_unique<CConstantBuffer>(
		4,
		static_cast<UINT>(indices.size()),
		indices.data(),
		D3D11_BIND_INDEX_BUFFER
		);
}

std::vector<ID3D11Buffer*> CModel::GetVertexBuffers() const noexcept
{
	CConstantBuffer* positionBuffer = m_positionBuffer.get();
	CConstantBuffer* uvTextureBuffer = m_uvCoordBuffer.get();
	CConstantBuffer* normalBuffer = m_normalBuffer.get();

	return vector<ID3D11Buffer*>
	{
		positionBuffer ? positionBuffer->GetBuffer() : nullptr,
			uvTextureBuffer ? uvTextureBuffer->GetBuffer() : nullptr,
			normalBuffer ? normalBuffer->GetBuffer() : nullptr,
	};
}

ID3D11Buffer* CModel::GetIndexBuffer() const noexcept
{
	CConstantBuffer* indexBuffer = m_indexBuffer.get();
	return indexBuffer ? indexBuffer->GetBuffer() : nullptr;
}

void CModel::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_worldTransformationGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_worldTransformationCPU));
	m_worldTransformationGPU->InitializeBuffer(device);
}

void CModel::Update(ID3D11DeviceContext* deviceContext, float dt)
{

}
