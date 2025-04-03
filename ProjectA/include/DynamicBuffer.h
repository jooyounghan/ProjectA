#pragma once

#ifndef DYNAMICBUFFER_H
#define DYNAMICBUFFER_H

#include "AUploadableBuffer.h"

namespace D3D11
{
	class D3D11MANAGER_API CDynamicBuffer : public AUploadableBuffer
	{
	public:
		CDynamicBuffer(
			UINT elementSize, 
			UINT arrayCount, 
			const void* cpuData,
			D3D11_BIND_FLAG bindFlag = D3D11_BIND_CONSTANT_BUFFER
		);
		~CDynamicBuffer() override = default;

	protected:
		D3D11_BIND_FLAG m_bindFlag;

	public:
		virtual D3D11_BUFFER_DESC CreateBufferDesc() noexcept override;
	};
}

#endif
