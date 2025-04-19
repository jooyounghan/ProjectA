#pragma once

#ifndef VERTEXSHADER_H
#define VERTEXSHADER_H

#include "AShader.h"

namespace D3D11
{
	class D3D11MANAGER_API CVertexShader : public AShader
	{
	public:
		CVertexShader(UINT inputLayoutCount);
		~CVertexShader() override;

	protected:
		Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
		UINT  m_inputLayoutCount;
		UINT  m_currentLayoutCount = 0;
		D3D11_INPUT_ELEMENT_DESC* m_inputElementDescs;
		
	protected:
		virtual void CreateShaderImpl(ID3D11Device* device) override;

	public:
		void AddInputLayoutElement(const D3D11_INPUT_ELEMENT_DESC& inputElementDesc);

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
