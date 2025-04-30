#pragma once
#include "ITexture.h"
#include "MacroUtilities.h"

template<typename ...IsTextureOption>
class Texture2DInstance : public ITexture2D, public IsTextureOption...
{
public:
	Texture2DInstance(
		UINT width, UINT height, UINT arraySize, 
		UINT mipLevels, UINT cpuAccessFlag, UINT miscFlagIn, 
		D3D11_USAGE usage, DXGI_FORMAT format,
		UINT bitLevel, UINT channelCount
	);
	Texture2DInstance(
		UINT width, UINT height, UINT arraySize,
		UINT mipLevels, UINT cpuAccessFlag, UINT miscFlagIn,
		D3D11_USAGE usage, DXGI_FORMAT format,
		UINT bitLevel, UINT channelCount,
		const UINT8* textureDataPerArray,
		const UINT* textureRowPitchPerArray
	);

public:
	static D3D11_BIND_FLAG GetBindFlags()
	{
		return (D3D11_BIND_FLAG)(IsTextureOption::GetBindFlag() | ...);
	};

protected:
	UINT m_bitLevel;
	UINT m_channelCount;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> m_texture2D;
	D3D11_TEXTURE2D_DESC m_texture2DDesc;
	D3D11_SUBRESOURCE_DATA m_subresourceData;

public:
	virtual ID3D11Texture2D* const GetTexture2D() const { return m_texture2D.Get(); }
	virtual D3D11_TEXTURE2D_DESC* const GetTexture2DDesc() { return &m_texture2DDesc; }

public:
	virtual void InitializeByOption(
		ID3D11Device* device, 
		ID3D11DeviceContext* deviceContext,
		ID3D11Resource* resource = nullptr 
	) override;
};

template<typename ...IsTextureOption>
inline Texture2DInstance<IsTextureOption...>::Texture2DInstance(
	UINT width, UINT height, UINT arraySize,
	UINT mipLevels, UINT cpuAccessFlag, UINT miscFlagIn,
	D3D11_USAGE usage, DXGI_FORMAT format,
	UINT bitLevel, UINT channelCount
)
	: m_bitLevel(bitLevel), m_channelCount(channelCount)
{
	ZeroMem(m_texture2DDesc);
	m_texture2DDesc.Width = width;
	m_texture2DDesc.Height = height;
	m_texture2DDesc.ArraySize = arraySize;
	m_texture2DDesc.MipLevels = mipLevels;
	m_texture2DDesc.BindFlags = Texture2DInstance<IsTextureOption...>::GetBindFlags();
	m_texture2DDesc.CPUAccessFlags = cpuAccessFlag;
	m_texture2DDesc.MiscFlags = miscFlagIn;
	m_texture2DDesc.SampleDesc.Count = 1;
	m_texture2DDesc.SampleDesc.Quality = 0;
	m_texture2DDesc.Usage = usage;
	m_texture2DDesc.Format = format;

	ZeroMem(m_subresourceData);
}

template<typename ...IsTextureOption>
inline Texture2DInstance<IsTextureOption...>::Texture2DInstance(
	UINT width, UINT height, UINT arraySize,
	UINT mipLevels, UINT cpuAccessFlag, UINT miscFlagIn,
	D3D11_USAGE usage, DXGI_FORMAT format,
	UINT bitLevel, UINT channelCount,
	const UINT8* textureDataPerArray,
	const UINT* textureRowPitchPerArray
)
	: m_bitLevel(bitLevel), m_channelCount(channelCount)
{
	ZeroMem(m_texture2DDesc);
	m_texture2DDesc.Width = width;
	m_texture2DDesc.Height = height;
	m_texture2DDesc.ArraySize = arraySize;
	m_texture2DDesc.MipLevels = mipLevels;
	m_texture2DDesc.BindFlags = Texture2DInstance<IsTextureOption...>::GetBindFlags();
	m_texture2DDesc.CPUAccessFlags = cpuAccessFlag;
	m_texture2DDesc.MiscFlags = miscFlagIn;
	m_texture2DDesc.SampleDesc.Count = 1;
	m_texture2DDesc.SampleDesc.Quality = 0;
	m_texture2DDesc.Usage = usage;
	m_texture2DDesc.Format = format;

	ZeroMem(m_subresourceData);
	m_subresourceData.pSysMem = textureDataPerArray;
	m_subresourceData.SysMemPitch = width * m_bitLevel * m_channelCount;
}

template<typename ...IsTextureOption>
inline void Texture2DInstance<IsTextureOption...>::InitializeByOption(
	ID3D11Device* device,
	ID3D11DeviceContext* deviceContext,
	ID3D11Resource* resource
)
{
	HRESULT hResult = device->CreateTexture2D(
		&m_texture2DDesc, 
		m_subresourceData.pSysMem ? &m_subresourceData : nullptr, 
		m_texture2D.GetAddressOf()
	);
	if (FAILED(hResult)) { throw std::exception("CreateTexture2D Failed"); }
	(IsTextureOption::InitializeByOption(device, deviceContext, m_texture2D.Get()), ...);
}

