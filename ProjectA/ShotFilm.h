#pragma once
#include "BaseFilm.h"
#include "DSVOption.h"

class CShotFilm : public CBaseFilm
{
public:
	CShotFilm(
		UINT width, 
		UINT height, 
		DXGI_FORMAT sceneFormat,
		DXGI_FORMAT depthStencilFormat,
		UINT bitLevel, 
		UINT channelCount,
		ID3D11RenderTargetView* backBufferRTV = nullptr
	);
	virtual ~CShotFilm() override = default;

protected:
	Texture2DInstance<D3D11::SRVOption, D3D11::DSVOption> m_depthStencil;

public:
	inline ID3D11DepthStencilView* GetFilmDSV() { return m_depthStencil.GetDSV(); }
	inline ID3D11ShaderResourceView* GetFilmDepthSRV() { return m_depthStencil.GetSRV(); }

protected:
	ID3D11RenderTargetView* m_backBufferRTV;
	Texture2DInstance<D3D11::SRVOption, D3D11::RTVOption> m_pingpongFilm;

protected:
	Texture2DInstance<D3D11::SRVOption, D3D11::RTVOption> m_luminanceCheckFilm;
	D3D11_VIEWPORT m_luminanceViewport;

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void ClearFilm(ID3D11DeviceContext* deviceContext) override;

public:
	virtual void Develop(ID3D11DeviceContext* deviceContext) override;
};

