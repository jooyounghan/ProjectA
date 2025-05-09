#pragma once
#include "AFilm.h"

#include <vector>

class BlurFilm : public AFilm
{
public:
	BlurFilm(
		size_t blurCount,
		UINT width,
		UINT height,
		DXGI_FORMAT sceneFormat,
		UINT bitLevel,
		UINT channelCount
	);
	virtual ~BlurFilm() override = default;

protected:
	size_t m_blurCount;
	std::vector<Texture2DInstance<D3D11::RTVOption, D3D11::SRVOption>> m_blurredFilms;

protected:
	std::vector<D3D11_VIEWPORT> m_blurredViewports;
	std::vector<ID3D11RenderTargetView*> m_blurredRTVs;
	std::vector<ID3D11ShaderResourceView*> m_blurrredSRVs;
	
public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void ClearFilm(ID3D11DeviceContext* deviceContext) override;

public:
	virtual void Blend(
		ID3D11DeviceContext* deviceContext, 
		ID3D11RenderTargetView* blendTarget, 
		const D3D11_VIEWPORT& blendTargetViewport
	) override;
	virtual void Develop(ID3D11DeviceContext* deviceContext) override;
};

