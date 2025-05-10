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
		static Microsoft::WRL::ComPtr<ID3D11BlendState> gBSAdditiveSS;
		static Microsoft::WRL::ComPtr<ID3D11BlendState> gBSAdditiveMS;
		static Microsoft::WRL::ComPtr<ID3D11BlendState> gBSAlphaWeightedAdditiveSS;
		static Microsoft::WRL::ComPtr<ID3D11BlendState> gBSAlphaWeightedAdditiveMS;
		static Microsoft::WRL::ComPtr<ID3D11BlendState> gBSAlphaSS;
		static Microsoft::WRL::ComPtr<ID3D11BlendState> gBSAlphaMS;
		static Microsoft::WRL::ComPtr<ID3D11BlendState> gBSPreMultipliedAlphaSS;
		static Microsoft::WRL::ComPtr<ID3D11BlendState> gBSPreMultipliedAlphaMS;
		static Microsoft::WRL::ComPtr<ID3D11BlendState> gBSMultiplicativeAlphaSS;
		static Microsoft::WRL::ComPtr<ID3D11BlendState> gBSMultiplicativeAlphaMS;

	public:
		static inline ID3D11BlendState* GetBSAdditiveSS() { return gBSAdditiveSS.Get(); }
		static inline ID3D11BlendState* GetBSAdditiveMS() { return gBSAdditiveMS.Get(); }
		static inline ID3D11BlendState* GetBSAlphaWeightedAdditiveSS() { return gBSAlphaWeightedAdditiveSS.Get(); }
		static inline ID3D11BlendState* GetBSAlphaWeightedAdditiveMS() { return gBSAlphaWeightedAdditiveMS.Get(); }
		static inline ID3D11BlendState* GetBSAlphaSS() { return gBSAlphaSS.Get(); }
		static inline ID3D11BlendState* GetBSAlphaMS() { return gBSAlphaMS.Get(); }
		static inline ID3D11BlendState* GetBSPreMultipliedAlphaSS() { return gBSPreMultipliedAlphaSS.Get(); }
		static inline ID3D11BlendState* GetBSPreMultipliedAlphaMS() { return gBSPreMultipliedAlphaMS.Get(); }
		static inline ID3D11BlendState* GetBSMultiplicativeAlphaSS() { return gBSMultiplicativeAlphaSS.Get(); }
		static inline ID3D11BlendState* GetBSMultiplicativeAlphaMS() { return gBSMultiplicativeAlphaMS.Get(); }
	};
}
#endif