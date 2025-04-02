#pragma once

#ifndef DEFERREDCONTEXT_H
#define DEFERREDCONTEXT_H

#include "D3D11DllHelper.h"

namespace D3D11
{
	class D3D11MANAGER_API CDeferredContext
	{
	public:
		CDeferredContext(ID3D11Device* const device);
		~CDeferredContext() noexcept = default;

	protected:
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_deferredContext;

	public:
		inline ID3D11DeviceContext* GetDeferredContext() noexcept { return m_deferredContext.Get(); }
		inline ID3D11DeviceContext* const* GetDeferredContextAddress()  noexcept { return m_deferredContext.GetAddressOf(); }

	public:
		ID3D11CommandList* FinishCommandList();
	};
}

#endif
