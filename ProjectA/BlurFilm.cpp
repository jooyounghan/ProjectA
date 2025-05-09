#include "BlurFilm.h"
#include "FilterCommonData.h"

#include "ConstantBuffer.h"
#include "GraphicsPSOObject.h"

#include "MacroUtilities.h"

#include <DirectXMath.h>

using namespace std;
using namespace D3D11;
using namespace DirectX;

BlurFilm::BlurFilm(
	size_t blurCount,
	float blurRadius,
	UINT width, 
	UINT height, 
	DXGI_FORMAT sceneFormat, 
	UINT bitLevel, 
	UINT channelCount
)
	: AFilm(width, height, sceneFormat, bitLevel, channelCount),
	m_blurCount(blurCount)
{
	ZeroMem(m_blurFilmPropertiesCPU);
	m_blurFilmPropertiesCPU.m_blurRadius = blurRadius;

	UINT blurWidth = width;
	UINT blurHeight = height;
	D3D11_VIEWPORT blurredViewport = m_viewport;

	m_blurredViewports.emplace_back(m_viewport);
	for (size_t blurIdx = 0; blurIdx < blurCount; ++blurIdx)
	{
		blurWidth = UINT(ceil(blurWidth / 2.f));
		blurHeight = UINT(ceil(blurHeight / 2.f));
		blurredViewport.Width = static_cast<FLOAT>(blurWidth);
		blurredViewport.Height = static_cast<FLOAT>(blurHeight);

		m_blurredFilms.emplace_back(blurWidth, blurHeight, 1, 1, NULL, NULL, D3D11_USAGE_DEFAULT, sceneFormat, 1, 4);
		m_blurredViewports.emplace_back(blurredViewport);
	}
}

void BlurFilm::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	AFilm::Initialize(device, deviceContext);

	m_blurFilmPropertiesGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_blurFilmPropertiesCPU));
	m_blurFilmPropertiesGPU->InitializeBuffer(device);

	for (UINT blurIdx = 0; blurIdx < m_blurCount; ++blurIdx)
	{
		m_blurredFilms[blurIdx].InitializeByOption(device, deviceContext);
	}

	m_blurredRTVs.reserve(m_blurCount + 1);
	m_blurrredSRVs.reserve(m_blurCount + 1);

	m_blurredRTVs.emplace_back(m_film.GetRTV());
	m_blurrredSRVs.emplace_back(m_film.GetSRV());

	for (auto& blurredFilm : m_blurredFilms)
	{
		m_blurredRTVs.emplace_back(blurredFilm.GetRTV());
		m_blurrredSRVs.emplace_back(blurredFilm.GetSRV());
	}
}

void BlurFilm::ClearFilm(ID3D11DeviceContext* deviceContext)
{
	AFilm::ClearFilm(deviceContext);

	constexpr FLOAT clearColor[4] = { 0.f, 0.f, 0.f, 0.f };
	for (UINT blurIdx = 0; blurIdx < m_blurCount; ++blurIdx)
	{
		deviceContext->ClearRenderTargetView(m_blurredFilms[blurIdx].GetRTV(), clearColor);
	}
}

void BlurFilm::Blend(
	ID3D11DeviceContext* deviceContext,
	AFilm* blendTargetFilm,
	const D3D11_VIEWPORT& blendTargetViewport
)
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

	CFilterCommonData::GFilterAdditivePSO->ApplyPSO(deviceContext);
	{
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		deviceContext->IASetVertexBuffers(0, 2, vertexBuffers, strides, offsets);
		deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, NULL);

		ID3D11ShaderResourceView* traceSRV = m_film.GetSRV();
		ID3D11ShaderResourceView* traceNullSRV = nullptr;
		ID3D11RenderTargetView* blendRTV = blendTargetFilm->GetFilmRTV();
		ID3D11RenderTargetView* blendNullRTV = nullptr;

		deviceContext->OMSetRenderTargets(1, &blendRTV, nullptr);
		deviceContext->RSSetViewports(1, &blendTargetViewport);
		deviceContext->PSSetShaderResources(0, 1, &traceSRV);

		deviceContext->DrawIndexedInstanced(
			static_cast<UINT>(CFilterCommonData::GFilterQuadIndices.size()),
			1, NULL, NULL, NULL
		);

		deviceContext->PSSetShaderResources(0, 1, &traceNullSRV);
		deviceContext->OMSetRenderTargets(1, &blendNullRTV, nullptr);


		deviceContext->IASetVertexBuffers(0, 2, vertexNullBuffers, nullStrides, nullOffsets);
		deviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, NULL);
	}
	CFilterCommonData::GFilterAdditivePSO->RemovePSO(deviceContext);
}

void BlurFilm::Develop(ID3D11DeviceContext* deviceContext)
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

	constexpr float blendColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

	CFilterCommonData::GFilterBlurPSO->ApplyPSO(deviceContext, blendColor, 0xFFFFFFFF);
	{
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		deviceContext->IASetVertexBuffers(0, 2, vertexBuffers, strides, offsets);
		deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, NULL);

		ID3D11ShaderResourceView* nullSRV = nullptr;
		ID3D11RenderTargetView* nullRTV = nullptr;

		ID3D11Buffer* blurFilterBuffer = m_blurFilmPropertiesGPU->GetBuffer();
		ID3D11Buffer* blurFilterNullBuffer = nullptr;

		deviceContext->PSSetConstantBuffers(2, 1, &blurFilterBuffer);

		for (size_t srvOffset = 0; srvOffset < m_blurCount; ++srvOffset)
		{
			size_t rtvOffset = srvOffset + 1;
			deviceContext->OMSetRenderTargets(1, &m_blurredRTVs[rtvOffset], nullptr);
			deviceContext->RSSetViewports(1, &m_blurredViewports[rtvOffset]);

			deviceContext->PSSetShaderResources(0, 1, &m_blurrredSRVs[srvOffset]);

			deviceContext->DrawIndexedInstanced(
				static_cast<UINT>(CFilterCommonData::GFilterQuadIndices.size()),
				1, NULL, NULL, NULL
			);

			deviceContext->PSSetShaderResources(0, 1, &nullSRV);
			deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);
		}

		for (size_t upIdx = 0; upIdx < m_blurCount; ++upIdx)
		{
			size_t srvOffset = m_blurrredSRVs.size() - 1 - upIdx;
			size_t rtvOffset = srvOffset - 1;
			deviceContext->OMSetRenderTargets(1, &m_blurredRTVs[rtvOffset], nullptr);
			deviceContext->RSSetViewports(1, &m_blurredViewports[rtvOffset]);

			deviceContext->PSSetShaderResources(0, 1, &m_blurrredSRVs[srvOffset]);

			deviceContext->DrawIndexedInstanced(
				static_cast<UINT>(CFilterCommonData::GFilterQuadIndices.size()),
				1, NULL, NULL, NULL
			);

			deviceContext->PSSetShaderResources(0, 1, &nullSRV);
			deviceContext->OMSetRenderTargets(1, &nullRTV, nullptr);
		}

		deviceContext->PSSetConstantBuffers(2, 1, &blurFilterNullBuffer);

		deviceContext->IASetVertexBuffers(0, 2, vertexNullBuffers, nullStrides, nullOffsets);
		deviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, NULL);
	}
	CFilterCommonData::GFilterBlurPSO->RemovePSO(deviceContext);
}
