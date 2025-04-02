#pragma once

#ifndef GENGINE_H
#define GENGINE_H

#include "D3D11DllHelper.h"
#include <DirectXMath.h>

#if defined(DEBUG) || defined(_DEBUG)
constexpr UINT				CreateDeviceFlags = D3D11_CREATE_DEVICE_DEBUG;
#else
constexpr UINT				CreateDeviceFlags = NULL;
#endif

namespace D3D11
{
    class D3D11MANAGER_API GEngine
    {
    private:
        GEngine() = default;
        GEngine(const GEngine& engine) = delete;
        GEngine(GEngine&& engine) = delete;

    public:
        ~GEngine() = default;

    public:
        static GEngine* GetInstance();

    protected:
        Microsoft::WRL::ComPtr<ID3D11Device>		m_device;
        Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deviceContext;
        Microsoft::WRL::ComPtr<IDXGISwapChain>		m_swapChain;

    public:
        inline ID3D11Device*                GetDevice() const noexcept { return m_device.Get(); }
        inline ID3D11DeviceContext*         GetDeviceContext() const noexcept { return m_deviceContext.Get(); }
        inline ID3D11Device* const*         GetDeviceAddress() const noexcept { return m_device.GetAddressOf(); }
        inline ID3D11DeviceContext* const*  GetDeviceContextAddress() const noexcept { return m_deviceContext.GetAddressOf(); }
        inline IDXGISwapChain*              GetSwapChain() const noexcept { return m_swapChain.Get(); }

    protected:
        UINT                    m_backBufferCount = NULL;
        DXGI_FORMAT             m_backBufferFormat = DXGI_FORMAT_UNKNOWN;
        DXGI_SWAP_CHAIN_FLAG    m_swapChainFlag = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

    public:
        inline const UINT&                  GetBackBufferCount() const noexcept { return m_backBufferCount; }
        inline const DXGI_FORMAT&           GetBackBufferFormat() const noexcept { return m_backBufferFormat; }
        inline const DXGI_SWAP_CHAIN_FLAG&  GetSwapChainFlag() const noexcept { return m_swapChainFlag; }

    protected:
        Microsoft::WRL::ComPtr<ID3D11Texture2D>         m_backBufferTexture;
        Microsoft::WRL::ComPtr<ID3D11RenderTargetView>  m_backBufferRTV;

    public:
        inline ID3D11Texture2D*                 GetBackBufferTexture() const noexcept { return m_backBufferTexture.Get(); }
        inline ID3D11RenderTargetView*          GetBackBufferRTV() const noexcept { return m_backBufferRTV.Get(); }
        inline ID3D11Texture2D* const*          GetBackBufferTextureAddress() const noexcept { return m_backBufferTexture.GetAddressOf(); }
        inline ID3D11RenderTargetView* const*   GetBackBufferRTVAddress() const noexcept { return m_backBufferRTV.GetAddressOf(); }

    public:
        void InitEngine(
            UINT widthIn,
            UINT heightIn,
            UINT refreshNumerator,
            UINT refreshDenominator,
            DXGI_FORMAT backBufferFormat,
            UINT backBufferCount,
            HWND wndHandle,
            DXGI_SWAP_EFFECT swapEffect,
            DXGI_SWAP_CHAIN_FLAG swapChainFlag
        );
        void ResizeSwapChain(UINT widthIn, UINT heightIn);

    private:
        void InitBackBuffer();
    };
}


#endif