#pragma once
#include "IUpdatable.h"
#include "Texture2DInstance.h"
#include "SRVOption.h"
#include "RTVOption.h"

#include <d3d11.h>

class AFilm : public IUpdatable
{
public:
	AFilm(UINT width, UINT height, DXGI_FORMAT sceneFormat, UINT bitLevel, UINT channelCount);
	virtual ~AFilm() = default;

protected:
	Texture2DInstance<D3D11::SRVOption, D3D11::RTVOption> m_film;

public:
	inline ID3D11RenderTargetView* GetFilmRTV() { return m_film.GetRTV(); }
	inline ID3D11ShaderResourceView* GetFilmSRV() { return m_film.GetSRV(); }

protected:
	D3D11_VIEWPORT m_viewport;

public:
	inline const D3D11_VIEWPORT& GetFilmViewPort() { return m_viewport; }
	inline float GetFilmWidth() const { return m_viewport.Width; }
	inline float GetFilmHeight() const { return m_viewport.Height; }
	inline float GetFilmAspectRatio() const { return m_viewport.Width / m_viewport.Height; }

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

public:
	virtual void ClearFilm(ID3D11DeviceContext* deviceContext);

public:
	virtual void Blend(
		ID3D11DeviceContext* deviceContext, 
		AFilm* blendTargetFilm,
		const D3D11_VIEWPORT& blendTargetViewport
	) {};
	virtual void Develop(ID3D11DeviceContext* deviceContext) {};
};