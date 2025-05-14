#pragma once
#ifndef RTVOPTION_H
#define RTVOPTION_H

#include "D3D11DllHelper.h"
#include "ITexture.h"

namespace D3D11
{
	class D3D11MANAGER_API RTVOption : public ITextureOption
	{
	public:
		RTVOption() = default;

	public:
		constexpr static D3D11_BIND_FLAG GetBindFlag()
		{
			return D3D11_BIND_RENDER_TARGET;
		}

	public:
		virtual void InitializeByOption(
			ID3D11Device* device,
			ID3D11DeviceContext* deviceContext,
			ID3D11Resource* resource
		) override;

	protected:
		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;

	public:
		ID3D11RenderTargetView* const GetRTV() const { return m_rtv.Get(); }

	public:
		void Swap(RTVOption& rtvOptionIn);

	private:
		static D3D11_RENDER_TARGET_VIEW_DESC GetRenderTargetViewDesc(const D3D11_TEXTURE2D_DESC& texture2dDesc);
		static DXGI_FORMAT GetRenderTargetViewFormatFromTextureFormat(const DXGI_FORMAT& textureFormat);
	};
}
#endif