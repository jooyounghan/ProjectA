#include "MotionBlurFilm.h"
#include "FilterCommonData.h"

#include "ConstantBuffer.h"
#include "ComputeShader.h"
#include "GraphicsPSOObject.h"

#include "MacroUtilities.h"
#include "DefineLinkedWithShader.h"

#include <DirectXMath.h>

using namespace std;
using namespace DirectX;
using namespace D3D11;

CMotionBlurFilm::CMotionBlurFilm(
	UINT samplingCount,
	float dissipationFactor,
	float maxMotionTrailLength,
	UINT width, 
	UINT height,
	DXGI_FORMAT sceneFormat,
	UINT bitLevel,
	UINT channelCount
)
	: CBaseFilm(width, height, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 4),
	m_motionBlurredFilm(width, height, 1, 1, NULL, NULL, D3D11_USAGE_DEFAULT, sceneFormat, bitLevel, channelCount)
{
	ZeroMem(m_motionBlurFilmPropertiesCPU);
	m_motionBlurFilmPropertiesCPU.m_samplingCount = samplingCount;
	m_motionBlurFilmPropertiesCPU.m_dissipationFactor = dissipationFactor;
	m_motionBlurFilmPropertiesCPU.m_maxMotionTrailLength = maxMotionTrailLength;
}

void CMotionBlurFilm::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	CBaseFilm::Initialize(device, deviceContext);
	
	m_motionBlurFilmPropertiesGPU = make_unique<CDynamicBuffer>(PASS_SINGLE(m_motionBlurFilmPropertiesCPU));
	m_motionBlurFilmPropertiesGPU->InitializeBuffer(device);

	m_motionBlurredFilm.InitializeByOption(device, deviceContext);
}

void CMotionBlurFilm::ClearFilm(ID3D11DeviceContext* deviceContext)
{
	CBaseFilm::ClearFilm(deviceContext);
	constexpr FLOAT clearColor[4] = { 0.f, 0.f, 0.f, 0.f };
	deviceContext->ClearRenderTargetView(m_motionBlurredFilm.GetRTV(), clearColor);
}


void CMotionBlurFilm::Blend(ID3D11DeviceContext* deviceContext, CBaseFilm* blendTargetFilm)
{
	CFilterCommonData::GFilterMotionBlurCS->SetShader(deviceContext);
	{
		ID3D11Buffer* motionBlurFilterBuffer = m_motionBlurFilmPropertiesGPU->GetBuffer();
		ID3D11Buffer* motionBlurFilterNullBuffer = nullptr;
		ID3D11ShaderResourceView* motionBlurSRVs[] = { m_film.GetSRV(), blendTargetFilm->GetFilmSRV() };
		ID3D11ShaderResourceView* motionBlurNullSRVs[] = { nullptr, nullptr };
		ID3D11UnorderedAccessView* motionBlurUAV = m_motionBlurredFilm.GetUAV();
		ID3D11UnorderedAccessView* motionBlurNullUAV = nullptr;
		UINT initialValue[] = { 0 };

		deviceContext->CSSetConstantBuffers(2, 1, &motionBlurFilterBuffer);
		deviceContext->CSSetShaderResources(0, 2, motionBlurSRVs);
		deviceContext->CSSetUnorderedAccessViews(0, 1, &motionBlurUAV, initialValue);

		deviceContext->Dispatch(UINT(ceil(m_viewport.Width / GroupTexWidth)), UINT(ceil(m_viewport.Width / GroupTexHeight)), 1);

		deviceContext->CSSetConstantBuffers(2, 1, &motionBlurFilterNullBuffer);
		deviceContext->CSSetShaderResources(0, 2, motionBlurNullSRVs);
		deviceContext->CSSetUnorderedAccessViews(0, 1, &motionBlurNullUAV, initialValue);
	}
	CFilterCommonData::GFilterMotionBlurCS->ResetShader(deviceContext);

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
		const D3D11_VIEWPORT& viewport = blendTargetFilm->GetFilmViewPort();
		deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		deviceContext->IASetVertexBuffers(0, 2, vertexBuffers, strides, offsets);
		deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, NULL);

		ID3D11ShaderResourceView* traceSRV = m_motionBlurredFilm.GetSRV();
		ID3D11ShaderResourceView* traceNullSRV = nullptr;
		ID3D11RenderTargetView* blendRTV = blendTargetFilm->GetFilmRTV();
		ID3D11RenderTargetView* blendNullRTV = nullptr;

		deviceContext->OMSetRenderTargets(1, &blendRTV, nullptr);
		deviceContext->RSSetViewports(1, &viewport);
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
