#pragma once

#ifndef SAMPLERSTATE_H
#define SAMPLERSTATE_H

#include "D3D11DllHelper.h"

#include <float.h>

namespace D3D11
{
	class D3D11MANAGER_API CSamplerState
	{
	public:
		CSamplerState(
			ID3D11Device* device,
			D3D11_FILTER filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR,
			D3D11_TEXTURE_ADDRESS_MODE addressU = D3D11_TEXTURE_ADDRESS_CLAMP,
			D3D11_TEXTURE_ADDRESS_MODE addressV = D3D11_TEXTURE_ADDRESS_CLAMP,
			D3D11_TEXTURE_ADDRESS_MODE addressW = D3D11_TEXTURE_ADDRESS_CLAMP,
			FLOAT mipLODBias = 0,
			UINT maxAnisotropy = 1,
			D3D11_COMPARISON_FUNC comparisonFunc = D3D11_COMPARISON_NEVER,
			const FLOAT* borderColor = nullptr,
			FLOAT minLOD = -FLT_MAX,
			FLOAT maxLOD = FLT_MAX
		);
		~CSamplerState() = default;

	private:
		Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;

	public:
		inline ID3D11SamplerState* GetSS() const noexcept { return m_samplerState.Get(); }

	public:
		static void InitializeSamplerState(ID3D11Device* device);

	private:
		static Microsoft::WRL::ComPtr<ID3D11SamplerState> gSSWrap;
		static Microsoft::WRL::ComPtr<ID3D11SamplerState> gSSClamp;
		static Microsoft::WRL::ComPtr<ID3D11SamplerState> gSSWrapLess;
		static Microsoft::WRL::ComPtr<ID3D11SamplerState> gSSClampLess;

	public:
		static inline ID3D11SamplerState* GetSSWrap() { return gSSWrap.Get(); }
		static inline ID3D11SamplerState* GetSSClamp() { return gSSClamp.Get(); }
		static inline ID3D11SamplerState* GetSSWrapLess() { return gSSWrapLess.Get(); }
		static inline ID3D11SamplerState* GetSSClampLess() { return gSSClampLess.Get(); }
	};
}

#endif