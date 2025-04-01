#pragma once
#include "ConstantBuffer.h"
#include "DynamicBuffer.h"
#include <DirectXMath.h>

class AField
{
public:
	AField(
		const DirectX::XMVECTOR& positionIn,
		const DirectX::XMVECTOR& angleIn,
		uint32_t width, 
		uint32_t height, 
		uint32_t depth
	);

protected:
	D3D11::CConstantBuffer m_vertexBuffer;
	D3D11::CConstantBuffer m_indexBuffer;

protected:
	uint32_t m_width;
	uint32_t m_height;
	uint32_t m_depth;

protected:
	struct
	{
		DirectX::XMMATRIX worldMatrix;
		DirectX::XMMATRIX worldInvTransposeMatrix;
	} m_fieldTransformationCPU;
	D3D11::CDynamicBuffer m_fieldTransformationGPU;
	bool m_isFieldTransformationChanged;

public:
	inline bool GetProjChanged() noexcept { return m_isFieldTransformationChanged; }
	inline ID3D11Buffer* GetViewBuffer() noexcept { return m_fieldTransformationGPU.GetBuffer(); }



};

