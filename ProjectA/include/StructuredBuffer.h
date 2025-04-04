#pragma once

#ifndef STRUCTUREDBUFFER_H
#define STRUCTUREDBUFFER_H

#include "ABuffer.h"

namespace D3D11
{
	class D3D11MANAGER_API CStructuredBuffer : public ABuffer
	{
	public:
		CStructuredBuffer(
			UINT elementSize, 
			UINT arrayCount, 
			const void* cpuData
		);
		~CStructuredBuffer() override = default;

	protected:
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_structuredSRV;
		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_structuredUAV;

	public:
		ID3D11ShaderResourceView* const GetSRV() const noexcept { return m_structuredSRV.Get(); }
		ID3D11UnorderedAccessView* const GetUAV() const noexcept { return m_structuredUAV.Get(); }

	protected:
		virtual D3D11_BUFFER_DESC CreateBufferDesc() noexcept override;
		virtual D3D11_SHADER_RESOURCE_VIEW_DESC CreateShaderResourceViewDesc() noexcept;
		virtual D3D11_UNORDERED_ACCESS_VIEW_DESC CreateUnorderedAccessViewDesc() noexcept;

	public:
		virtual void InitializeBuffer(ID3D11Device* const device) override;
	};
}

#endif
