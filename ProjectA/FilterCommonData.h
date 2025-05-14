#pragma once

#include <d3d11.h>
#include <vector>
#include <memory>
#include <DirectXMath.h>

namespace D3D11
{
	class CVertexShader;
	class CPixelShader;
	class CComputeShader;
	class CGraphicsPSOObject;

	class CConstantBuffer;
}

class CFilterCommonData
{
public:
	static void Intialize(ID3D11Device* device);

public:
	static const std::vector<DirectX::XMFLOAT3> GFilterQuadPosition;
	static const std::vector<DirectX::XMFLOAT2> GFilterQuadUVCoords;
	static const std::vector<uint32_t> GFilterQuadIndices;

public:
	static std::unique_ptr<D3D11::CConstantBuffer> GFilterQuadPositionBuffer;
	static std::unique_ptr<D3D11::CConstantBuffer> GFilterQuadUVCoordBuffer;
	static std::unique_ptr<D3D11::CConstantBuffer> GFilterQuadIndexBuffer;

public:
	static std::unique_ptr<D3D11::CVertexShader> GFilterVS;

public:
	static std::unique_ptr<D3D11::CPixelShader> GFilterTracePS;
	static std::unique_ptr<D3D11::CPixelShader> GFilterBlurPS;
	static std::unique_ptr<D3D11::CPixelShader> GFilterGetLuminancePS;
	static std::unique_ptr<D3D11::CPixelShader> GFilterGammaCorrectionPS;

public:
	static std::unique_ptr<D3D11::CComputeShader> GFilterCalculateLogLuminanceCS;

public:
	static std::unique_ptr<D3D11::CGraphicsPSOObject> GFilterAdditivePSO;
	static std::unique_ptr<D3D11::CGraphicsPSOObject> GFilterBlurPSO;
	static std::unique_ptr<D3D11::CGraphicsPSOObject> GFilterGetLuminancePSO;
	static std::unique_ptr<D3D11::CGraphicsPSOObject> GFilterGammaCorrectionPSO;
};

