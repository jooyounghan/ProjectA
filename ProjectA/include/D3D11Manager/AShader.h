#pragma once

#ifndef ASHADER_H
#define ASHADER_H

#include "D3D11DllHelper.h"

#include <string>
#include <exception>

#include <d3dcompiler.h>

#if defined(_DEBUG)
constexpr UINT ShaderCompileFlag = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
constexpr UINT ShaderCompileFlag = NULL;
#endif

namespace D3D11
{
	class D3D11MANAGER_API AShader
	{
	public:
		AShader() = default;
		virtual ~AShader() = default;

	public:
		void CreateShader(
			const std::wstring& shaderPath,
			const D3D_SHADER_MACRO* shaderMacro,
			const std::string& entryPoint,
			const std::string& targetVersion,
			ID3D11Device* device
		);

	protected:
		virtual void CreateShaderImpl(ID3D11Device* device) = 0;

	protected:
		Microsoft::WRL::ComPtr<ID3DBlob> m_shaderByteCode;
		Microsoft::WRL::ComPtr<ID3DBlob> m_errorByteCode;

	protected:
		UINT m_samplerStateCount = 0;

	public:
		virtual void SetShader(ID3D11DeviceContext* deviceContext) const noexcept = 0;
		virtual void ResetShader(ID3D11DeviceContext* deviceContext) const noexcept = 0;
		virtual void SetSamplerState(
			ID3D11DeviceContext* deviceContext,
			ID3D11SamplerState* const* samplerStates,
			UINT samplerStatesCount
		) noexcept;
		virtual void ResetSamplerState(ID3D11DeviceContext* deviceContext) const noexcept = 0;
	};
}
#endif