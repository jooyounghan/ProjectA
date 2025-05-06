#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <type_traits>

template <class T>
concept IsTextureOption = requires (T option, ID3D11Device * device, ID3D11DeviceContext * deviceContext, ID3D11Resource * resource)
{
	T::GetBindFlag();
	option.InitializeByOption(device, deviceContext, resource);
	option.Swap(option);
};

class ITexture2D
{
public:
	virtual ID3D11Texture2D* const GetTexture2D() const = 0;
	virtual D3D11_TEXTURE2D_DESC* const GetTexture2DDesc() = 0;
};

class ITextureOption
{
public:
	virtual void InitializeByOption(
		ID3D11Device* device,
		ID3D11DeviceContext* deviceContext,
		ID3D11Resource* resource
	) = 0;
};