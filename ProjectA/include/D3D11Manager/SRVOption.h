#pragma once
#ifndef SRVOPTION_H
#define SRVOPTION_H

#include "D3D11DllHelper.h"
#include "ITexture.h"

namespace D3D11
{
	class D3D11MANAGER_API SRVOption : public ITextureOption
	{
	public:
		SRVOption() = default;

	protected:
		constexpr static D3D11_BIND_FLAG GetBindFlag()
		{
			return D3D11_BIND_SHADER_RESOURCE;
		}

	protected:
		virtual void InitializeByOption(
			ID3D11Device* device,
			ID3D11DeviceContext* deviceContext,
			ID3D11Resource* resource
		) override;

	protected:
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_srv;

	public:
		inline ID3D11ShaderResourceView* const GetSRV() const { return m_srv.Get(); }

	private:
		static D3D11_SHADER_RESOURCE_VIEW_DESC GetShaderResourceViewDesc(const D3D11_TEXTURE2D_DESC& texture2dDesc);
		static DXGI_FORMAT GetShaderResourceViewFormatFromTextureFormat(const DXGI_FORMAT& textureFormat);
	};
}

#endif
