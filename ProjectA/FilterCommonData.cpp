#include "FilterCommonData.h"

#include "VertexShader.h"
#include "PixelShader.h"
#include "ComputeShader.h"
#include "GraphicsPSOObject.h"

#include "ConstantBuffer.h"

#include "RasterizerState.h"
#include "BlendState.h"
#include "DepthStencilState.h"
#include "SamplerState.h"

#include "ModelFactory.h"

using namespace std;
using namespace DirectX;
using namespace D3D11;

const vector<XMFLOAT3> CFilterCommonData::GFilterQuadPosition = ModelFactory::CreateQuadPositions();
const vector<XMFLOAT2> CFilterCommonData::GFilterQuadUVCoords = ModelFactory::CreateQuadUVCoords();
const vector<UINT> CFilterCommonData::GFilterQuadIndices = ModelFactory::CreateQuadIndices();

unique_ptr<CConstantBuffer> CFilterCommonData::GFilterQuadPositionBuffer = nullptr;
unique_ptr<CConstantBuffer> CFilterCommonData::GFilterQuadUVCoordBuffer = nullptr;
unique_ptr<CConstantBuffer> CFilterCommonData::GFilterQuadIndexBuffer = nullptr;

unique_ptr<CVertexShader> CFilterCommonData::GFilterVS;

unique_ptr<CPixelShader> CFilterCommonData::GFilterTracePS;
unique_ptr<CPixelShader> CFilterCommonData::GFilterBlurPS;
unique_ptr<CComputeShader> CFilterCommonData::GFilterMotionBlurCS;
unique_ptr<CPixelShader> CFilterCommonData::GFilterGammaCorrectionPS;

unique_ptr<CGraphicsPSOObject> CFilterCommonData::GFilterAdditivePSO;
unique_ptr<CGraphicsPSOObject> CFilterCommonData::GFilterBlurPSO;
unique_ptr<CGraphicsPSOObject> CFilterCommonData::GFilterGammaCorrectionPSO;

void CFilterCommonData::Intialize(ID3D11Device* device)
{
	GFilterQuadPositionBuffer = make_unique<CConstantBuffer>(
		static_cast<UINT>(sizeof(XMFLOAT3)),
		static_cast<UINT>(GFilterQuadPosition.size()),
		GFilterQuadPosition.data(), 
		D3D11_BIND_VERTEX_BUFFER
	);
	GFilterQuadPositionBuffer->InitializeBuffer(device);

	GFilterQuadUVCoordBuffer = make_unique<CConstantBuffer>(
		static_cast<UINT>(sizeof(XMFLOAT2)),
		static_cast<UINT>(GFilterQuadUVCoords.size()), 
		GFilterQuadUVCoords.data(),
		D3D11_BIND_VERTEX_BUFFER
	);
	GFilterQuadUVCoordBuffer->InitializeBuffer(device);

	GFilterQuadIndexBuffer = make_unique<CConstantBuffer>(
		static_cast<UINT>(sizeof(UINT)), 
		static_cast<UINT>(GFilterQuadIndices.size()),
		GFilterQuadIndices.data(), 
		D3D11_BIND_INDEX_BUFFER
	);
	GFilterQuadIndexBuffer->InitializeBuffer(device);

	GFilterVS = make_unique<CVertexShader>(2);
	GFilterVS->AddInputLayoutElement(
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	);
	GFilterVS->AddInputLayoutElement(
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	);
	GFilterVS->CreateShader(L"./FilterVS.hlsl", nullptr, "main", "vs_5_0", device);
	
	GFilterTracePS = make_unique<CPixelShader>();
	GFilterTracePS->CreateShader(L"./FilterTracePS.hlsl", nullptr, "main", "ps_5_0", device);

	GFilterBlurPS = make_unique<CPixelShader>();
	GFilterBlurPS->CreateShader(L"./FilterBlurPS.hlsl", nullptr, "main", "ps_5_0", device);

	GFilterMotionBlurCS = make_unique<CComputeShader>();
	GFilterMotionBlurCS->CreateShader(L"./FilterMotionBlurCS.hlsl", nullptr, "main", "cs_5_0", device);

	GFilterGammaCorrectionPS = make_unique<CPixelShader>();
	GFilterGammaCorrectionPS->CreateShader(L"./FilterGammaCorrectionPS.hlsl", nullptr, "main", "ps_5_0", device);

	static ID3D11SamplerState* samplerStates[] = { CSamplerState::GetSSClamp() };

	GFilterAdditivePSO = make_unique<CGraphicsPSOObject>(
		GFilterVS.get(),
		nullptr,
		nullptr,
		nullptr,
		GFilterTracePS.get(),
		CRasterizerState::GetRSSolidCWSS(),
		CBlendState::GetBSAdditiveSS(),
		CDepthStencilState::GetDSSDisabled(),
		samplerStates,
		1
	);

	GFilterBlurPSO = make_unique<CGraphicsPSOObject>(
		GFilterVS.get(),
		nullptr,
		nullptr,
		nullptr,
		GFilterBlurPS.get(),
		CRasterizerState::GetRSSolidCWSS(),
		nullptr,
		CDepthStencilState::GetDSSDisabled(),
		samplerStates,
		1
	);

	GFilterGammaCorrectionPSO = make_unique<CGraphicsPSOObject>(
		GFilterVS.get(),
		nullptr,
		nullptr,
		nullptr,
		GFilterGammaCorrectionPS.get(),
		CRasterizerState::GetRSSolidCWSS(),
		nullptr,
		CDepthStencilState::GetDSSDisabled(),
		samplerStates,
		1
	);
}
