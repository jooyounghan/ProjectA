#include "AFilm.h"
#include "MacroUtilities.h"

using namespace D3D11;

AFilm::AFilm(
	UINT width, UINT height, 
	DXGI_FORMAT sceneFormat,
	UINT bitLevel, 
	UINT channelCount
)
	: m_film(width, height, 1, 1, NULL, NULL, D3D11_USAGE_DEFAULT, sceneFormat, bitLevel, channelCount)
{
	ZeroMem(m_viewport);
	m_viewport.TopLeftX = 0.f;
	m_viewport.TopLeftY = 0.f;
	m_viewport.Width = static_cast<FLOAT>(width);
	m_viewport.Height = static_cast<FLOAT>(height);
	m_viewport.MinDepth = 0.f;
	m_viewport.MaxDepth = 1.f;
}

void AFilm::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_film.InitializeByOption(device, deviceContext);
}

void AFilm::Update(ID3D11DeviceContext* deviceContext, float dt)
{
}

void AFilm::ClearFilm(ID3D11DeviceContext* deviceContext)
{
	constexpr FLOAT clearColor[4] = { 0.f, 0.f, 0.f, 1.f };
	deviceContext->ClearRenderTargetView(m_film.GetRTV(), clearColor);
}
