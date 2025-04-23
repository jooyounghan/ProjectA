#pragma once

#ifndef DYNAMICBUFFER_H
#define DYNAMICBUFFER_H

#include "ABuffer.h"

namespace D3D11
{
	class D3D11MANAGER_API CDynamicBuffer : public ABuffer
	{
	public:
		CDynamicBuffer(
			UINT elementSize, 
			UINT arrayCount, 
			const void* cpuData,
			UINT bindFlag = D3D11_BIND_CONSTANT_BUFFER
		);
		~CDynamicBuffer() override = default;

	protected:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_stagingBuffer;
		UINT m_bindFlag;

	protected:
		virtual D3D11_BUFFER_DESC CreateBufferDesc() noexcept override;

	public:
		inline ID3D11Buffer* GetStagingBuffer() const noexcept { return m_stagingBuffer.Get(); }
		virtual void InitializeBuffer(ID3D11Device* const device) override;

	public:
		void Stage(ID3D11DeviceContext* const deviceContext);
		void Upload(ID3D11DeviceContext* const deviceContext) noexcept;

	public:
		void StageNthElement(
			ID3D11DeviceContext* const deviceContext,
			UINT* elementIndices,
			UINT elementIndicesCount
		);
		void UploadNthElement(
			ID3D11DeviceContext* const deviceContext,
			UINT* elementIndices,
			UINT elementIndicesCount
		);
	};
}

#endif
