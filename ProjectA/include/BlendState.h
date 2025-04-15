#pragma once

#ifndef BLENDSTATE_H
#define BLENDSTATE_H

#include "D3D11DllHelper.h"

namespace D3D11
{
	class D3D11MANAGER_API CBlendState
	{
	public:
		CBlendState(
			ID3D11Device* device,
			BOOL alphaToCoverageEnable,
			UINT renderTargetBlendDescCount,
			D3D11_RENDER_TARGET_BLEND_DESC* renderTargetBlendDescs
		);
		~CBlendState() = default;

	private:
		Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendState;

	public:
		inline ID3D11BlendState* GetRS() const noexcept { return m_blendState.Get(); }

	public:
		static void InitializeDefaultBlendStates(ID3D11Device* device);

	private:
		static Microsoft::WRL::ComPtr<ID3D11BlendState> gBSAccumulate;
		static Microsoft::WRL::ComPtr<ID3D11BlendState> gBSAccumulateWithAlpha;
		static Microsoft::WRL::ComPtr<ID3D11BlendState> gBSAlphaBlend;

	public:
		static inline ID3D11BlendState* GetBSAccumulate() { return gBSAccumulate.Get(); }
		static inline ID3D11BlendState* GetBSAccumulateWithAlpha() { return gBSAccumulateWithAlpha.Get(); }
		static inline ID3D11BlendState* GetBSAlphaBlend() { return gBSAlphaBlend.Get(); }
	};
}
#endif