#pragma once
#include "BaseFilm.h"
#include "DSVOption.h"
#include "DynamicBuffer.h"
#include "StructuredBuffer.h"

#include <memory>

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

protected:
	std::unique_ptr<D3D11::CStructuredBuffer> m_luminanceHistogramSet;
	std::unique_ptr<D3D11::CStructuredBuffer> m_adaptedLuminance;

protected:
	struct
	{
		float minLogLum;
		float maxLogLum;
		float dummy[2];
	} m_luminanceFilterPropertiesCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_luminanceFilterPropertiesGPU;

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void ClearFilm(ID3D11DeviceContext* deviceContext) override;

public:
	virtual void Develop(ID3D11DeviceContext* deviceContext) override;
};

