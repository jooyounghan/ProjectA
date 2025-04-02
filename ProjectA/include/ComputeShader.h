#pragma once

#ifndef COMPUTESHADER_H
#define COMPUTESHADER_H

#include "AShader.h"

namespace D3D11
{
	class D3D11MANAGER_API CComputeShader : public AShader
	{
	public:
		CComputeShader() = default;
		~CComputeShader() override = default;


	protected:
		Microsoft::WRL::ComPtr<ID3D11ComputeShader> m_computeShader;

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

