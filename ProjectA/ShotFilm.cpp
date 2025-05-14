#include "ShotFilm.h"
#include "FilterCommonData.h"
#include "DefineLinkedWithShader.h"
#include "MacroUtilities.h"

#include "ConstantBuffer.h"
#include "GraphicsPSOObject.h"
#include "ComputeShader.h"
#include <DirectXMath.h>

using namespace std;
using namespace D3D11;
using namespace DirectX;

CShotFilm::CShotFilm(
	UINT width, 
	UINT height,
	DXGI_FORMAT sceneFormat,
	DXGI_FORMAT depthStencilFormat,
	UINT bitLevel, 
	UINT channelCount,
	ID3D11RenderTargetView* backBufferRTV
)
	: CBaseFilm(width, height, sceneFormat, bitLevel, channelCount),
	m_depthStencil(width, height, 1, 1, NULL, NULL, D3D11_USAGE_DEFAULT, depthStencilFormat, 1, 4),
	m_backBufferRTV(backBufferRTV),
	m_pingpongFilm(width, height, 1, 1, NULL, NULL, D3D11_USAGE_DEFAULT, sceneFormat, bitLevel, channelCount),
	m_luminanceCheckFilm(GroupTexWidth * GroupTexWidth, GroupTexHeight * GroupTexHeight, 1, 1, NULL, NULL, D3D11_USAGE_DEFAULT, DXGI_FORMAT_R32_TYPELESS, 4, 1)
{
	ZeroMem(m_luminanceViewport);
	m_luminanceViewport.TopLeftX = 0.f;
	m_luminanceViewport.TopLeftY = 0.f;
	m_luminanceViewport.Width = static_cast<FLOAT>(64);
	m_luminanceViewport.Height = static_cast<FLOAT>(64);
	m_luminanceViewport.MinDepth = 0.f;
	m_luminanceViewport.MaxDepth = 1.f;

	ZeroMem(m_luminanceFilterPropertiesCPU);
	m_luminanceFilterPropertiesCPU.minLogLum = -10.f;
	m_luminanceFilterPropertiesCPU.maxLogLum = 2.f;
}

void CShotFilm::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	CBaseFilm::Initialize(device, deviceContext);
	m_depthStencil.InitializeByOption(device, deviceContext);
	if (m_backBufferRTV == nullptr)
	{
		m_pingpongFilm.InitializeByOption(device, deviceContext);
	}
	m_luminanceCheckFilm.InitializeByOption(device, deviceContext);

	m_luminanceHistogramSet = make_unique<CStructuredBuffer>(
		4, 
		GroupTexWidth * GroupTexHeight,
		nullptr
	);
	m_luminanceHistogramSet->InitializeBuffer(device);

	constexpr UINT clearValues[4] = { 0, 0, 0, 0 };

	m_adaptedLuminance = make_unique<CStructuredBuffer>(
		4, 1, nullptr
		);
	m_adaptedLuminance->InitializeBuffer(device);

	deviceContext->ClearUnorderedAccessViewUint(m_luminanceHistogramSet->GetUAV(), clearValues);
	deviceContext->ClearUnorderedAccessViewUint(m_adaptedLuminance->GetUAV(), clearValues);

	m_luminanceFilterPropertiesGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_luminanceFilterPropertiesCPU));
	m_luminanceFilterPropertiesGPU->InitializeBuffer(device);
}

void CShotFilm::ClearFilm(ID3D11DeviceContext* deviceContext)
{
	constexpr FLOAT clearColor[4] = { 0.f, 0.f, 0.f, 0.f };

	CBaseFilm::ClearFilm(deviceContext);

	deviceContext->ClearDepthStencilView(
		m_depthStencil.GetDSV(), 
		D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
		1.f, 0x00
	);

	if (m_backBufferRTV == nullptr)
	{
		deviceContext->ClearRenderTargetView(m_pingpongFilm.GetRTV(), clearColor);
	}
	deviceContext->ClearRenderTargetView(m_luminanceCheckFilm.GetRTV(), clearColor);
}

void CShotFilm::Develop(ID3D11DeviceContext* deviceContext)
{
	ID3D11Buffer* vertexBuffers[] = {
		CFilterCommonData::GFilterQuadPositionBuffer->GetBuffer(),
		CFilterCommonData::GFilterQuadUVCoordBuffer->GetBuffer()
	};
	ID3D11Buffer* vertexNullBuffers[] = { nullptr, nullptr };
	ID3D11Buffer* indexBuffer = CFilterCommonData::GFilterQuadIndexBuffer->GetBuffer();

	UINT strides[] = { static_cast<UINT>(sizeof(XMFLOAT3)), static_cast<UINT>(sizeof(XMFLOAT2)) };
	UINT nullStrides[] = { NULL, NULL };
	UINT offsets[] = { 0, 0 };
	UINT nullOffsets[] = { NULL, NULL };

	CFilterCommonData::GFilterGetLuminancePSO->ApplyPSO(deviceContext);
	{
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		deviceContext->IASetVertexBuffers(0, 2, vertexBuffers, strides, offsets);
		deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, NULL);

		ID3D11RenderTargetView* luminanceCheckRTV = m_luminanceCheckFilm.GetRTV();
		ID3D11RenderTargetView* luminanceCheckNullRTV = nullptr;
		ID3D11ShaderResourceView* filmSRV = m_film.GetSRV();
		ID3D11ShaderResourceView* nullFilmSRV = nullptr;

		deviceContext->OMSetRenderTargets(1, &luminanceCheckRTV, nullptr);
		deviceContext->RSSetViewports(1, &m_luminanceViewport);
		deviceContext->PSSetShaderResources(0, 1, &filmSRV);

		deviceContext->DrawIndexedInstanced(
			static_cast<UINT>(CFilterCommonData::GFilterQuadIndices.size()),
			1, NULL, NULL, NULL
		);

		deviceContext->PSSetShaderResources(0, 1, &nullFilmSRV);
		deviceContext->OMSetRenderTargets(1, &luminanceCheckNullRTV, nullptr);

		deviceContext->IASetVertexBuffers(0, 2, vertexNullBuffers, nullStrides, nullOffsets);
		deviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, NULL);
	}
	CFilterCommonData::GFilterGetLuminancePSO->RemovePSO(deviceContext);

	ID3D11Buffer* luminanceFilterBuffer = m_luminanceFilterPropertiesGPU->GetBuffer();
	ID3D11Buffer* luminanceFilterNullBuffer = nullptr;
	
	deviceContext->CSSetConstantBuffers(2, 1, &luminanceFilterBuffer);
	CFilterCommonData::GFilterSetLogLuminanceHistogramCS->SetShader(deviceContext);
	{
		ID3D11ShaderResourceView* luminanceCheckSRV = m_luminanceCheckFilm.GetSRV();
		ID3D11ShaderResourceView* luminanceCheckNullSRV = nullptr;
		ID3D11UnorderedAccessView* logLuminanceHistogramUAV = m_luminanceHistogramSet->GetUAV();
		ID3D11UnorderedAccessView* logLuminanceHistogramNullUAV = nullptr;
		UINT initUAVValue = NULL;

		deviceContext->CSSetShaderResources(0, 1, &luminanceCheckSRV);
		deviceContext->CSSetUnorderedAccessViews(0, 1, &logLuminanceHistogramUAV, &initUAVValue);
		deviceContext->Dispatch(GroupTexWidth, GroupTexWidth, 1);
		deviceContext->CSSetShaderResources(0, 1, &luminanceCheckNullSRV);
		deviceContext->CSSetUnorderedAccessViews(0, 1, &logLuminanceHistogramNullUAV, &initUAVValue);
	}
	CFilterCommonData::GFilterSetLogLuminanceHistogramCS->ResetShader(deviceContext);

	CFilterCommonData::GFilterGetAdaptedLuminanceCS->SetShader(deviceContext);
	{
		ID3D11UnorderedAccessView* adaptedLuminanceUavs[] = { m_luminanceHistogramSet->GetUAV(), m_adaptedLuminance->GetUAV() };
		ID3D11UnorderedAccessView* adaptedLuminanceNullUavs[] = { nullptr, nullptr };
		UINT initUAVValue[] = { NULL, NULL };

		deviceContext->CSSetUnorderedAccessViews(0, 2, adaptedLuminanceUavs, initUAVValue);
		deviceContext->Dispatch(1, 1, 1);
		deviceContext->CSSetUnorderedAccessViews(0, 2, adaptedLuminanceNullUavs, initUAVValue);
	}
	CFilterCommonData::GFilterGetAdaptedLuminanceCS->ResetShader(deviceContext);
	deviceContext->CSSetConstantBuffers(2, 1, &luminanceFilterNullBuffer);

	CFilterCommonData::GFilterGammaCorrectionPSO->ApplyPSO(deviceContext);
	{
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		deviceContext->IASetVertexBuffers(0, 2, vertexBuffers, strides, offsets);
		deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, NULL);

		ID3D11RenderTargetView* gammaCorrectionRTV = m_backBufferRTV ? m_backBufferRTV : m_pingpongFilm.GetRTV();
		ID3D11RenderTargetView* gammaCorrectionNullRTV = nullptr;
		ID3D11ShaderResourceView* gammaCorrectionSrvs[] = { m_film.GetSRV(), m_adaptedLuminance->GetSRV() };
		ID3D11ShaderResourceView* gammaCorrectionNullSrvs[] = { nullptr, nullptr };

		deviceContext->OMSetRenderTargets(1, &gammaCorrectionRTV, nullptr);
		deviceContext->RSSetViewports(1, &m_viewport);
		deviceContext->PSSetShaderResources(0, 2, gammaCorrectionSrvs);

		deviceContext->DrawIndexedInstanced(
			static_cast<UINT>(CFilterCommonData::GFilterQuadIndices.size()),
			1, NULL, NULL, NULL
		);

		deviceContext->PSSetShaderResources(0, 2, gammaCorrectionNullSrvs);
		deviceContext->OMSetRenderTargets(1, &gammaCorrectionNullRTV, nullptr);


		deviceContext->IASetVertexBuffers(0, 2, vertexNullBuffers, nullStrides, nullOffsets);
		deviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, NULL);
	}
	CFilterCommonData::GFilterGammaCorrectionPSO->RemovePSO(deviceContext);

	if (m_backBufferRTV == nullptr)
	{
		m_film.Swap(m_pingpongFilm);
	}
}
