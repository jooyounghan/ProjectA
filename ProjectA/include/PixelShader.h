#pragma once

#ifndef PIXELSHADER_H
#define PIXELSHADER_H

#include "AShader.h"

namespace D3D11
{
	class D3D11MANAGER_API CPixelShader : public AShader
	{
	public:
		CPixelShader() = default;
		~CPixelShader() override = default;


	protected:
		Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;

	protected:
		virtual void CreateShaderImpl(ID3D11Device* device) override;

	public:
		virtual void SetShader(ID3D11DeviceContext* deviceContext) const noexcept override;
		virtual void ResetShader(ID3D11DeviceContext* deviceContext) const noexcept override;
		virtual void SetSamplerState(
			ID3D11DeviceContext* deviceContext,
			ID3D11SamplerState* const* samplerStates,
			UINT samplerStatesCount
		)  noexcept override;
		virtual void ResetSamplerState(ID3D11DeviceContext* deviceContext) const noexcept override;

	};
}

#endif