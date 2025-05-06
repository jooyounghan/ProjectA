#pragma once
#ifndef UAVOPTION_H
#define UAVOPTION_H

#include "D3D11DllHelper.h"
#include "ITexture.h"

namespace D3D11
{
	class D3D11MANAGER_API UAVOption : public ITextureOption
	{
	public:
		UAVOption() = default;

	public:
		constexpr static D3D11_BIND_FLAG GetBindFlag()
		{
			return D3D11_BIND_UNORDERED_ACCESS;
		}

	public:
		virtual void InitializeByOption(
			ID3D11Device* device,
			ID3D11DeviceContext* deviceContext,
			ID3D11Resource* resource
		) override;

	protected:
		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_uav;

	public:
		ID3D11UnorderedAccessView* GetUAV() const { return m_uav.Get(); }

	public:
		void Swap(UAVOption& uavOptionIn);
	};
}
#endif
