#pragma once
#include "ITexture.h"

class DSVOption : public ITextureOption
{
public:
	DSVOption() = default;

protected:
	constexpr static D3D11_BIND_FLAG GetBindFlag()
	{
		return D3D11_BIND_DEPTH_STENCIL;
	}

protected:
	virtual void InitializeByOption(
		ID3D11Device* device, 
		ID3D11DeviceContext* deviceContext,
		ID3D11Resource* resource
	) override;

protected:
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_dsv;

public:
	ID3D11DepthStencilView* GetDSV() const { return m_dsv.Get(); }

private:
	static D3D11_DEPTH_STENCIL_VIEW_DESC GetDepthStencilViewDesc(const D3D11_TEXTURE2D_DESC& texture2dDesc);
	static DXGI_FORMAT GetDepthStencilViewFormatFromTextureFormat(const DXGI_FORMAT& textureFormat);
};