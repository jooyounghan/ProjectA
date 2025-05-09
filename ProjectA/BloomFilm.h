#pragma once
#include "AFilm.h"
#include "DynamicBuffer.h"

#include <vector>
#include <memory>


class CBloomFilm : public AFilm
{
public:
	CBloomFilm(
		size_t blurCount,
		float blurRadius,
		UINT width,
		UINT height,
		DXGI_FORMAT sceneFormat,
		UINT bitLevel,
		UINT channelCount
	);
	virtual ~CBloomFilm() override = default;

protected:
	size_t m_blurCount;
	std::vector<Texture2DInstance<D3D11::RTVOption, D3D11::SRVOption>> m_blurredFilms;

protected:
	struct
	{
		float m_blurRadius;
		float dummy[3];
	} m_bloomFilmPropertiesCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_bloomFilmPropertiesGPU;


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
		AFilm* blendTargetFilm
	) override;
	virtual void Develop(ID3D11DeviceContext* deviceContext) override;
};

