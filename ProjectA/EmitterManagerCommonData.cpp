#include "EmitterManagerCommonData.h"

#include "MacroUtilities.h"

#include "ComputeShader.h"
#include "VertexShader.h"
#include "GeometryShader.h"
#include "PixelShader.h"
#include "GraphicsPSOObject.h"

#include "ConstantBuffer.h"
#include "IndirectBuffer.h"
#include "AppendBuffer.h"
#include "StructuredBuffer.h"

#include "RasterizerState.h"
#include "BlendState.h"
#include "DepthStencilState.h"
#include "SamplerState.h"

#include "ModelFactory.h"

#include "ParticleStructure.h"


using namespace std;
using namespace DirectX;
using namespace D3D11;

unique_ptr<CComputeShader> CEmitterManagerCommonData::GInitializeParticleSetCS[EmitterTypeCount];
unique_ptr<CComputeShader> CEmitterManagerCommonData::GParticleInitialSourceCS[EmitterTypeCount];
unique_ptr<CComputeShader> CEmitterManagerCommonData::GParticleRuntimeSourceCS[EmitterTypeCount];
unique_ptr<CComputeShader> CEmitterManagerCommonData::GCaculateDispatchArgsCS = make_unique<CComputeShader>();
unique_ptr<CComputeShader> CEmitterManagerCommonData::GPrefixSumLocalHistogramDispatchArgsCS = make_unique<CComputeShader>();

unique_ptr<CComputeShader> CEmitterManagerCommonData::GCaculateParticleForceCS[EmitterTypeCount];

unique_ptr<CComputeShader> CEmitterManagerCommonData::GSpriteSetLocalHistogramCS = make_unique<CComputeShader>();
unique_ptr<CComputeShader> CEmitterManagerCommonData::GSpritePrefixSumLocalHistogramCS = make_unique<CComputeShader>();
unique_ptr<CComputeShader> CEmitterManagerCommonData::GSpritePrefixSumGlobalHistogramCS = make_unique<CComputeShader>();
unique_ptr<CComputeShader> CEmitterManagerCommonData::GSpriteSortCS = make_unique<CComputeShader>();

unique_ptr<CVertexShader> CEmitterManagerCommonData::GParticleDrawVS[EmitterTypeCount];
unique_ptr<CGeometryShader> CEmitterManagerCommonData::GParticleDrawGS[EmitterTypeCount];
unique_ptr<CPixelShader> CEmitterManagerCommonData::GParticleDrawPS[EmitterTypeCount];
unique_ptr<CGraphicsPSOObject> CEmitterManagerCommonData::GDrawParticlePSO[EmitterTypeCount];

const vector<XMFLOAT3> CEmitterManagerCommonData::GEmitterBoxPositions = ModelFactory::CreateBoxPositions(XMVectorSet(1.f, 1.f, 1.f, 0.f));
const vector<XMFLOAT3> CEmitterManagerCommonData::GEmitterBoxNormals = ModelFactory::CreateBoxNormals();
const vector<UINT> CEmitterManagerCommonData::GEmitterBoxIndices = ModelFactory::CreateBoxIndices();
unique_ptr<CVertexShader> CEmitterManagerCommonData::GEmitterDrawVS;
unique_ptr<CPixelShader> CEmitterManagerCommonData::GEmitterDrawPS[EmitterTypeCount];
unique_ptr<CGraphicsPSOObject> CEmitterManagerCommonData::GDrawEmitterPSO[EmitterTypeCount];
unique_ptr<CConstantBuffer> CEmitterManagerCommonData::GEmitterPositionBuffer = nullptr;
unique_ptr<CConstantBuffer> CEmitterManagerCommonData::GEmitterNormalBuffer = nullptr;
unique_ptr<CConstantBuffer> CEmitterManagerCommonData::GEmitterIndexBuffer = nullptr;

void CEmitterManagerCommonData::Intialize(ID3D11Device* device)
{
#pragma region 세이더 매크로
	const D3D_SHADER_MACRO emitterTypeMacros[EmitterTypeCount][2] =
	{
		{
			{ "PARTICLE_EMITTER", nullptr },
			{ nullptr, nullptr }
		},
		{
			{ "SPRITE_EMITTER", nullptr },
			{ nullptr, nullptr }
		},
		{
			{ "RIBBON_EMITTER", nullptr },
			{ nullptr, nullptr }
		},
		{
			{ "MESH_EMITTER", nullptr },
			{ nullptr, nullptr }
		}
	};

	const D3D_SHADER_MACRO sourceMacro[][2] = {
		{
			{ "INITIAL_SOURCE", nullptr },
			{ nullptr, nullptr }
		},
		{
			{ "RUNTIME_SOURCE", nullptr },
			{ nullptr, nullptr }
		},
	};
#pragma endregion

#pragma region 입자 초기화 관련 CS
	for (size_t idx = 0; idx < EmitterTypeCount; ++idx)
	{
		GInitializeParticleSetCS[idx] = make_unique<CComputeShader>();
		GInitializeParticleSetCS[idx]->CreateShader(L"./InitializeParticleSetCS.hlsl", emitterTypeMacros[idx], "main", "cs_5_0", device);
	}
#pragma endregion

#pragma region 입자 방출 관련 CS
	for (size_t idx = 0; idx < EmitterTypeCount; ++idx)
	{
		D3D_SHADER_MACRO emitterSourceMacro[3];
		memcpy(&emitterSourceMacro[0], emitterTypeMacros[idx], sizeof(D3D_SHADER_MACRO));
		memcpy(&emitterSourceMacro[1], sourceMacro[0], sizeof(D3D_SHADER_MACRO) * 2);
		GParticleInitialSourceCS[idx] = make_unique<CComputeShader>();
		GParticleInitialSourceCS[idx]->CreateShader(L"./ParticleSourceCS.hlsl", emitterSourceMacro, "main", "cs_5_0", device);

		memcpy(&emitterSourceMacro[1], sourceMacro[1], sizeof(D3D_SHADER_MACRO) * 2);
		GParticleRuntimeSourceCS[idx] = make_unique<CComputeShader>();
		GParticleRuntimeSourceCS[idx]->CreateShader(L"./ParticleSourceCS.hlsl", emitterSourceMacro, "main", "cs_5_0", device);

	}
#pragma endregion

#pragma region Indirect 인자 계산 관련 CS
	const D3D_SHADER_MACRO dispatchMacro[2] = {
		{ "DISPATCH_RADIX_SORT", nullptr },
		{ nullptr, nullptr }
	};

	GCaculateDispatchArgsCS->CreateShader(L"./CalculateDispatchArgsCS.hlsl", nullptr, "main", "cs_5_0", device);
	GPrefixSumLocalHistogramDispatchArgsCS->CreateShader(L"./CalculateDispatchArgsCS.hlsl", dispatchMacro, "main", "cs_5_0", device);
#pragma endregion

#pragma region 입자 시뮬레이션 관련 CS
	for (size_t idx = 0; idx < EmitterTypeCount; ++idx)
	{
		GCaculateParticleForceCS[idx] = make_unique<CComputeShader>();
		GCaculateParticleForceCS[idx]->CreateShader(L"./CalculateParticleForceCS.hlsl", emitterTypeMacros[idx], "main", "cs_5_0", device);
	}
#pragma endregion

#pragma region 스프라이트 소팅 관련 CS
	GSpriteSetLocalHistogramCS->CreateShader(L"./SpriteSetLocalHistogramCS.hlsl", nullptr, "main", "cs_5_0", device);
	GSpritePrefixSumLocalHistogramCS->CreateShader(L"./SpritePrefixSumLocalHistogramCS.hlsl", nullptr, "main", "cs_5_0", device);
	GSpritePrefixSumGlobalHistogramCS->CreateShader(L"./SpritePrefixSumGlobalHistogramCS.hlsl", nullptr, "main", "cs_5_0", device);
	GSpriteSortCS->CreateShader(L"./SpriteSortCS.hlsl", nullptr, "main", "cs_5_0", device);
#pragma endregion

#pragma region 입자 그리기 관련 PSO
	for (size_t idx = 0; idx < EmitterTypeCount; ++idx)
	{
		GParticleDrawVS[idx] = make_unique<CVertexShader>(0);
		GParticleDrawGS[idx] = make_unique<CGeometryShader>();
		GParticleDrawPS[idx] = make_unique<CPixelShader>();
	}

	static ID3D11SamplerState* samplerStates[] = { CSamplerState::GetSSWrap() };
	ID3D11BlendState* blendStates[EmitterTypeCount] = 
	{
		CBlendState::GetBSAlphaWeightedAdditiveSS(),
		CBlendState::GetBSAlphaSS(),
		CBlendState::GetBSAlphaSS(),
		CBlendState::GetBSAlphaSS()
	};

	ID3D11DepthStencilState* depthStencilStates[EmitterTypeCount] = 
	{
		CDepthStencilState::GetDSSReadOnly(),
		CDepthStencilState::GetDSSReadOnly(),
		CDepthStencilState::GetDSSReadWrite(),
		CDepthStencilState::GetDSSReadWrite()
	};

	for (size_t idx = 0; idx < EmitterTypeCount; ++idx)
	{
		GParticleDrawVS[idx]->CreateShader(L"./ParticleDrawVS.hlsl", emitterTypeMacros[idx], "main", "vs_5_0", device);
		GParticleDrawGS[idx]->CreateShader(L"./ParticleDrawGS.hlsl", emitterTypeMacros[idx], "main", "gs_5_0", device);
		GParticleDrawPS[idx]->CreateShader(L"./ParticleDrawPS.hlsl", emitterTypeMacros[idx], "main", "ps_5_0", device);

		GDrawParticlePSO[idx] = make_unique<CGraphicsPSOObject>(
			GParticleDrawVS[idx].get(),
			nullptr,
			nullptr,
			GParticleDrawGS[idx].get(),
			GParticleDrawPS[idx].get(),
			CRasterizerState::GetRSSolidCWSS(),
			blendStates[idx],
			depthStencilStates[idx],
			samplerStates,
			1
		);
	}
#pragma endregion

#pragma region 이미터 그리기 PSO
	GEmitterDrawVS = make_unique<CVertexShader>(6);

	for (size_t idx = 0; idx < EmitterTypeCount; ++idx)
	{
		GEmitterDrawPS[idx] = make_unique<CPixelShader>();
	}

	GEmitterPositionBuffer = make_unique<CConstantBuffer>(
		12, static_cast<UINT>(GEmitterBoxPositions.size()), GEmitterBoxPositions.data(), D3D11_BIND_VERTEX_BUFFER
	);
	GEmitterPositionBuffer->InitializeBuffer(device);

	GEmitterNormalBuffer = make_unique<CConstantBuffer>(
		12, static_cast<UINT>(GEmitterBoxNormals.size()), GEmitterBoxNormals.data(), D3D11_BIND_VERTEX_BUFFER
	);
	GEmitterNormalBuffer->InitializeBuffer(device);


	GEmitterIndexBuffer = make_unique<CConstantBuffer>(
		4, static_cast<UINT>(GEmitterBoxIndices.size()), GEmitterBoxIndices.data(), D3D11_BIND_INDEX_BUFFER
		);
	GEmitterIndexBuffer->InitializeBuffer(device);

	GEmitterDrawVS->AddInputLayoutElement(
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	);
	GEmitterDrawVS->AddInputLayoutElement(
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	);
	GEmitterDrawVS->AddInputLayoutElement(
		{ "INSTANCE_WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	);
	GEmitterDrawVS->AddInputLayoutElement(
		{ "INSTANCE_WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	);
	GEmitterDrawVS->AddInputLayoutElement(
		{ "INSTANCE_WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	);
	GEmitterDrawVS->AddInputLayoutElement(
		{ "INSTANCE_WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 2, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	);

	GEmitterDrawVS->CreateShader(L"./EmitterDrawVS.hlsl", nullptr, "main", "vs_5_0", device);


	ID3D11RasterizerState* rasterizerStates[] = {
		CRasterizerState::GetRSSolidCWSS(),
		CRasterizerState::GetRSWireframeCWSS(),
		CRasterizerState::GetRSWireframeCWSS(),
		CRasterizerState::GetRSWireframeCWSS()
	};

	for (size_t idx = 0; idx < EmitterTypeCount; ++idx)
	{
		GEmitterDrawPS[idx]->CreateShader(L"./EmitterDrawPS.hlsl", emitterTypeMacros[idx], "main", "ps_5_0", device);
		GDrawEmitterPSO[idx] = make_unique<CGraphicsPSOObject>(
			GEmitterDrawVS.get(),
			nullptr,
			nullptr,
			nullptr,
			GEmitterDrawPS[idx].get(),
			rasterizerStates[idx],
			nullptr,
			CDepthStencilState::GetDSSReadWrite(),
			nullptr,
			0
		);
	}
#pragma endregion
}