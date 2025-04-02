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
			const void* cpuData
		);
		~CDynamicBuffer() override = default;

	public:
		virtual D3D11_BUFFER_DESC CreateBufferDesc() noexcept override;
	};
}

#endif
