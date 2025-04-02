#pragma once

#ifndef INDIRECTBUFFER_H
#define INDIRECTBUFFER_H

#include "ABuffer.h" 

namespace D3D11
{
	class D3D11MANAGER_API CIndirectBuffer : public ABuffer
	{
	public:
		CIndirectBuffer(UINT arrayCount, const D3D11_DRAW_INSTANCED_INDIRECT_ARGS* cpuData);
		~CIndirectBuffer() override = default;

	protected:
		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_indirectUAV;

	public:
		ID3D11UnorderedAccessView* const GetUAV() const noexcept { return m_indirectUAV.Get(); }

	public:
		virtual D3D11_BUFFER_DESC CreateBufferDesc() noexcept override;
		virtual D3D11_UNORDERED_ACCESS_VIEW_DESC CreateUnorderedAccessViewDesc() noexcept;
		virtual void InitializeBuffer(ID3D11Device* const device) override;
	};
}

#endif
