#pragma once

#ifndef GRAPHICSPSOOBJEC_H
#define GRAPHICSPSOOBJEC_H

#include "D3D11DllHelper.h"

namespace D3D11
{
	class AShader;
	class CVertexShader;
	class CHullShader;
	class CDomainShader;
	class CGeometryShader;
	class CPixelShader;
}

namespace D3D11
{
	class D3D11MANAGER_API CGraphicsPSOObject
	{
	public:
		CGraphicsPSOObject(
			CVertexShader* vertexShader,
			CHullShader* hullShader,
			CDomainShader* domainShader,
			CGeometryShader* geometryShader,
			CPixelShader* pixelShader,
			ID3D11RasterizerState* rasterizerState,
			ID3D11BlendState* blendState,
			ID3D11DepthStencilState* depthStencilState,
			ID3D11SamplerState* const* samplerStates,
			UINT samplerStateCount
		) noexcept;

	protected:
		AShader* m_shaders[5]{ nullptr, nullptr, nullptr, nullptr, nullptr };
		ID3D11RasterizerState* m_rasterizerState = nullptr;
		ID3D11BlendState* m_blendState = nullptr;
		ID3D11DepthStencilState* m_depthStencilState = nullptr;
		ID3D11SamplerState* const* m_samplerStates = nullptr;
		const UINT m_samplerStateCount;

	public:
		void ApplyPSO(
			ID3D11DeviceContext* deviceContext,
			const FLOAT blendFactor[4] = NULL,
			UINT blendSamplerMask = 0xFFFFFFFF,
			UINT stencilRef = NULL
		);

		void RemovePSO(
			ID3D11DeviceContext* deviceContext
		);
	};
}
#endif