#pragma once
#include <d3d11.h>
#include <wrl/client.h>

namespace D3D11
{
	#ifdef D3D11MANAGER_EXPORTS
	#define D3D11MANAGER_API __declspec(dllexport)
	#else
	#define D3D11MANAGER_API __declspec(dllimport)
	#endif
}

template class D3D11MANAGER_API Microsoft::WRL::ComPtr<ID3D11Device>;
template class D3D11MANAGER_API Microsoft::WRL::ComPtr<ID3D11DeviceContext>;
template class D3D11MANAGER_API Microsoft::WRL::ComPtr<IDXGISwapChain>;

template class D3D11MANAGER_API Microsoft::WRL::ComPtr<ID3D11Buffer>;

template class D3D11MANAGER_API Microsoft::WRL::ComPtr<ID3D11Texture2D>;
template class D3D11MANAGER_API Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>;
template class D3D11MANAGER_API Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView>;
template class D3D11MANAGER_API Microsoft::WRL::ComPtr<ID3D11RenderTargetView>;

template class D3D11MANAGER_API Microsoft::WRL::ComPtr<ID3DBlob>;
template class D3D11MANAGER_API Microsoft::WRL::ComPtr<ID3D11ComputeShader>;
template class D3D11MANAGER_API Microsoft::WRL::ComPtr<ID3D11VertexShader>;
template class D3D11MANAGER_API Microsoft::WRL::ComPtr<ID3D11HullShader>;
template class D3D11MANAGER_API Microsoft::WRL::ComPtr<ID3D11GeometryShader>;
template class D3D11MANAGER_API Microsoft::WRL::ComPtr<ID3D11DomainShader>;
template class D3D11MANAGER_API Microsoft::WRL::ComPtr<ID3D11PixelShader>;

template class D3D11MANAGER_API Microsoft::WRL::ComPtr<ID3D11InputLayout>;
template class D3D11MANAGER_API Microsoft::WRL::ComPtr<ID3D11DepthStencilState>;
template class D3D11MANAGER_API Microsoft::WRL::ComPtr<ID3D11BlendState>;
template class D3D11MANAGER_API Microsoft::WRL::ComPtr<ID3D11RasterizerState>;
template class D3D11MANAGER_API Microsoft::WRL::ComPtr<ID3D11SamplerState>;

