#pragma once

#ifndef AUPLOADABLEBUFFER_H
#define AUPLOADABLEBUFFER_H

#include "ABuffer.h"

namespace D3D11
{
	class D3D11MANAGER_API AUploadableBuffer : public ABuffer
	{
	public:
		AUploadableBuffer(UINT elementSize, UINT arrayCount, const void* cpuData);
		~AUploadableBuffer() override = default;

	protected:
		Microsoft::WRL::ComPtr<ID3D11Buffer> m_stagingBuffer;

	public:
		inline ID3D11Buffer* GetStagingBuffer() const noexcept { return m_stagingBuffer.Get(); }

	public:
		virtual void InitializeBuffer(ID3D11Device* const device) override;

	public:
		void Stage(ID3D11DeviceContext* const deviceContext);
		void Upload(ID3D11DeviceContext* const deviceContext) noexcept;
	};
}
#endif
