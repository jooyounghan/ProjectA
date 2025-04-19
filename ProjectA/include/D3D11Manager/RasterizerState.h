#pragma once

#ifndef RASTERIZERSTATE_H
#define RASTERIZERSTATE_H

#include "D3D11DllHelper.h"

namespace D3D11
{
	class D3D11MANAGER_API CRasterizerState
	{
	public:
		CRasterizerState(
			ID3D11Device* device,
            D3D11_FILL_MODE fillMode = D3D11_FILL_SOLID,
            D3D11_CULL_MODE cullMode = D3D11_CULL_BACK,
            BOOL frontCounterClockwise = FALSE,
            BOOL multisampleEnable = FALSE,
            INT depthBias = D3D11_DEFAULT_DEPTH_BIAS,
            FLOAT depthBiasClamp = D3D11_DEFAULT_DEPTH_BIAS_CLAMP,
            FLOAT slopeScaledDepthBias = D3D11_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
            BOOL depthClipEnable = TRUE,
            BOOL scissorEnable = FALSE,
            BOOL antialiasedLineEnable = FALSE
		);
        ~CRasterizerState() = default;

    private:
        Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rasterizerState;

    public:
        inline ID3D11RasterizerState* GetRS() const noexcept { return m_rasterizerState.Get(); }

    public:
        static void InitializeDefaultRasterizerStates(ID3D11Device* device);

    private:
        static Microsoft::WRL::ComPtr<ID3D11RasterizerState> gRSSolidCWSS;
        static Microsoft::WRL::ComPtr<ID3D11RasterizerState> gRSWireframeCWSS;
        static Microsoft::WRL::ComPtr<ID3D11RasterizerState> gRSSolidCCWSS;
        static Microsoft::WRL::ComPtr<ID3D11RasterizerState> gRSWireframeCCWSS;

    public:
        static inline ID3D11RasterizerState* GetRSSolidCWSS() { return gRSSolidCWSS.Get(); }
        static inline ID3D11RasterizerState* GetRSWireframeCWSS() { return gRSWireframeCWSS.Get(); }
        static inline ID3D11RasterizerState* GetRSSolidCCWSS() { return gRSSolidCCWSS.Get(); }
        static inline ID3D11RasterizerState* GetRSWireframeCCWSS() { return gRSWireframeCCWSS.Get(); }

    private:
        static Microsoft::WRL::ComPtr<ID3D11RasterizerState> gRSSolidCWMS;
        static Microsoft::WRL::ComPtr<ID3D11RasterizerState> gRSWireframeCWMS;
        static Microsoft::WRL::ComPtr<ID3D11RasterizerState> gRSSolidCCWMS;
        static Microsoft::WRL::ComPtr<ID3D11RasterizerState> gRSWireframeCCWMS;

    public:
        static inline ID3D11RasterizerState* GetRSSolidCWMS() { return gRSSolidCWMS.Get(); }
        static inline ID3D11RasterizerState* GetRSWireframeCWMS() { return gRSWireframeCWMS.Get(); }
        static inline ID3D11RasterizerState* GetRSSolidCCWMS() { return gRSSolidCCWMS.Get(); }
        static inline ID3D11RasterizerState* GetRSWireframeCCWMS() { return gRSWireframeCCWMS.Get(); }
    };
}

#endif
