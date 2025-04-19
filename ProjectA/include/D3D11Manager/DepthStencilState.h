#pragma once

#ifndef DEPTHSTENCILSTATE_H
#define DEPTHSTENCILSTATE_H

#include "D3D11DllHelper.h"

#include <d3d11.h>
#include <wrl/client.h>


namespace D3D11
{
	class D3D11MANAGER_API CDepthStencilState
	{
	public:
		CDepthStencilState(
			ID3D11Device* device,
            BOOL depthEnable = TRUE,
            D3D11_DEPTH_WRITE_MASK depthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL,
            D3D11_COMPARISON_FUNC depthFunc = D3D11_COMPARISON_LESS,
            BOOL stencilEnable = FALSE,
            UINT8 stencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK,
            UINT8 stencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK,
            D3D11_STENCIL_OP frontStencilFailOp = D3D11_STENCIL_OP_KEEP,
            D3D11_STENCIL_OP frontStencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
            D3D11_STENCIL_OP frontStencilPassOp = D3D11_STENCIL_OP_KEEP,
            D3D11_COMPARISON_FUNC frontStencilFunc = D3D11_COMPARISON_ALWAYS,
            D3D11_STENCIL_OP backStencilFailOp = D3D11_STENCIL_OP_KEEP,
            D3D11_STENCIL_OP backStencilDepthFailOp = D3D11_STENCIL_OP_KEEP,
            D3D11_STENCIL_OP backStencilPassOp = D3D11_STENCIL_OP_KEEP,
            D3D11_COMPARISON_FUNC backStencilFunc = D3D11_COMPARISON_ALWAYS
		);
		~CDepthStencilState() = default;

    private:
        Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilState;

    public:
        inline ID3D11DepthStencilState* GetDSS() const noexcept { return m_depthStencilState.Get(); }

    public:
        static void InitializeDefaultDepthStencilState(ID3D11Device* device);

    private:
        static Microsoft::WRL::ComPtr<ID3D11DepthStencilState> gDSSDraw;
        static Microsoft::WRL::ComPtr<ID3D11DepthStencilState> gDSSMasking;
        static Microsoft::WRL::ComPtr<ID3D11DepthStencilState> gDSSConditionalDraw;

    public:
        static inline ID3D11DepthStencilState* GetDSDraw() { return gDSSDraw.Get(); }
        static inline ID3D11DepthStencilState* GetDSMasking() { return gDSSMasking.Get(); }
        static inline ID3D11DepthStencilState* GetDSConditionalDraw() { return gDSSConditionalDraw.Get(); }
	};
}

#endif