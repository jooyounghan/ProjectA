#pragma once
#include "ABuffer.h"
#include "MacroUtilities.h"

#include <exception>

namespace D3D11
{
	template<typename T>
	class D3D11MANAGER_API CIndirectBuffer : public ABuffer
	{
	public:
		CIndirectBuffer(UINT arrayCount, const T* cpuData)
			: ABuffer(sizeof(T), arrayCount, cpuData)
		{

		}
		~CIndirectBuffer() override = default;

	protected:
		Microsoft::WRL::ComPtr<ID3D11UnorderedAccessView> m_indirectUAV;

	public:
		ID3D11UnorderedAccessView* const GetUAV() const noexcept { return m_indirectUAV.Get(); }

	protected:
		virtual D3D11_BUFFER_DESC CreateBufferDesc() noexcept override
		{
			D3D11_BUFFER_DESC bufferDesc;
			ZeroMem(bufferDesc);

			bufferDesc.ByteWidth = m_elementSize * m_arrayCount;
			bufferDesc.Usage = D3D11_USAGE_DEFAULT;
			bufferDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
			bufferDesc.CPUAccessFlags = NULL;
			bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
			return bufferDesc;
		}
		virtual D3D11_UNORDERED_ACCESS_VIEW_DESC CreateUnorderedAccessViewDesc() noexcept
		{
			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc;
			ZeroMem(uavDesc);

			uavDesc.Format = DXGI_FORMAT_R32_UINT;
			uavDesc.ViewDimension = D3D11_UAV_DIMENSION_BUFFER;
			uavDesc.Buffer.NumElements = m_elementSize / 4;
			return uavDesc;
		}

	public:
		virtual void InitializeBuffer(ID3D11Device* const device) override
		{
			D3D11_SUBRESOURCE_DATA initialData = GetSubResourceData();
			D3D11_BUFFER_DESC bufferDesc = CreateBufferDesc();
			HRESULT hResult = device->CreateBuffer(&bufferDesc, m_cpuData ? &initialData : nullptr, m_buffer.GetAddressOf());
			if (FAILED(hResult)) throw std::exception("CreateBuffer With InitializeBuffer Failed");

			D3D11_UNORDERED_ACCESS_VIEW_DESC uavDesc = CreateUnorderedAccessViewDesc();
			hResult = device->CreateUnorderedAccessView(m_buffer.Get(), &uavDesc, m_indirectUAV.GetAddressOf());
			if (FAILED(hResult)) throw std::exception("CreateUnorderedAccessView For InitializeBuffer Failed");
		}
	};
}

