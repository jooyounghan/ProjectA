#pragma once

#ifndef ABUFFER_H
#define ABUFFER_H

#include "D3D11DllHelper.h"

#include <wrl/client.h>
#include <d3d11.h>

#include <exception>

namespace D3D11
{
	class D3D11MANAGER_API ABuffer
	{
	public:
		ABuffer(UINT elementSize, UINT arrayCount, const void* cpuDataIn);
		virtual ~ABuffer() { m_buffer.Reset(); };

	protected:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_buffer;

	protected:
		UINT m_elementSize;
		UINT m_arrayCount;
		const void* m_cpuData;

	public:
		D3D11_SUBRESOURCE_DATA GetSubResourceData() const noexcept;
		inline ID3D11Buffer* GetBuffer() const noexcept { return m_buffer.Get(); }

	public:
		virtual D3D11_BUFFER_DESC CreateBufferDesc() noexcept = 0;
		virtual void InitializeBuffer(ID3D11Device* const device) = 0;
	};
}

#endif

