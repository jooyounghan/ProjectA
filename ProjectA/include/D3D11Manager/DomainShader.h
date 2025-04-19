#pragma once

#ifndef DOMAINSHADER_H
#define DOMAINSHADER_H

#include "AShader.h"

namespace D3D11
{
	class D3D11MANAGER_API CDomainShader : public AShader
	{
	public:
		CDomainShader() = default;
		~CDomainShader() override = default;


	protected:
		Microsoft::WRL::ComPtr<ID3D11DomainShader> m_domainShader;

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