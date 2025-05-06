#pragma once
#ifndef DSVOPTION_H
#define DSVOPTION_H

#include "D3D11DllHelper.h"
#include "ITexture.h"

namespace D3D11
{
	class D3D11MANAGER_API DSVOption : public ITextureOption
	{
	public:
		DSVOption() = default;

	public:
		constexpr static D3D11_BIND_FLAG GetBindFlag()
		{
			return D3D11_BIND_DEPTH_STENCIL;
		}

	public:
		virtual void InitializeByOption(
			ID3D11Device* device,
			ID3D11DeviceContext* deviceContext,
			ID3D11Resource* resource
		) override;

	protected:
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsv;

	public:
		ID3D11DepthStencilView* GetDSV() const { return m_dsv.Get(); }

	public:
		void Swap(DSVOption& dsvOptionIn);

	private:
		static D3D11_DEPTH_STENCIL_VIEW_DESC GetDepthStencilViewDesc(const D3D11_TEXTURE2D_DESC& texture2dDesc);
		static DXGI_FORMAT GetDepthStencilViewFormatFromTextureFormat(const DXGI_FORMAT& textureFormat);
	};
}
#endif