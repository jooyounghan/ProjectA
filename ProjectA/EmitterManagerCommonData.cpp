#include "EmitterManagerCommonData.h"

#include "MacroUtilities.h"

#include "ComputeShader.h"
#include "VertexShader.h"
#include "GeometryShader.h"
#include "PixelShader.h"
#include "GraphicsPSOObject.h"

#include "ConstantBuffer.h"
#include "StructuredBuffer.h"

#include "RasterizerState.h"
#include "BlendState.h"
#include "DepthStencilState.h"

#include "ModelFactory.h"

#include "ParticleStructure.h"


using namespace std;
using namespace DirectX;
using namespace D3D11;

UINT CEmitterManagerCommonData::GParticleMaxCount = 0;
unique_ptr<CStructuredBuffer> CEmitterManagerCommonData::GTotalParticles;
unique_ptr<CAppendBuffer> CEmitterManagerCommonData::GDeathIndexSet;

unique_ptr<CComputeShader> CEmitterManagerCommonData::GInitializeParticleSetCS = make_unique<CComputeShader>();
unique_ptr<CComputeShader> CEmitterManagerCommonData::GParticleInitialSourceCS = make_unique<CComputeShader>();
unique_ptr<CComputeShader> CEmitterManagerCommonData::GParticleRuntimeSourceCS = make_unique<CComputeShader>();
unique_ptr<CComputeShader> CEmitterManagerCommonData::GCalcualteIndirectArgCS = make_unique<CComputeShader>();
unique_ptr<CComputeShader> CEmitterManagerCommonData::GCaculateParticleForceCS = make_unique<CComputeShader>();

unique_ptr<CVertexShader> CEmitterManagerCommonData::GParticleDrawVS = make_unique<CVertexShader>(0);
unique_ptr<CGeometryShader> CEmitterManagerCommonData::GParticleDrawGS = make_unique<CGeometryShader>();
unique_ptr<CPixelShader> CEmitterManagerCommonData::GParticleDrawPS = make_unique<CPixelShader>();
unique_ptr<CGraphicsPSOObject> CEmitterManagerCommonData::GDrawParticlePSO = nullptr;

const vector<XMFLOAT3> CEmitterManagerCommonData::GEmitterBoxPositions = ModelFactory::CreateBoxPositions(XMVectorSet(1.f, 1.f, 1.f, 0.f));
const vector<UINT> CEmitterManagerCommonData::GEmitterBoxIndices = ModelFactory::CreateIndices();
unique_ptr<CVertexShader> CEmitterManagerCommonData::GEmitterDrawVS;
unique_ptr<CPixelShader> CEmitterManagerCommonData::GEmitterDrawPS[EmitterTypeCount];
unique_ptr<CGraphicsPSOObject> CEmitterManagerCommonData::GDrawEmitterPSO[EmitterTypeCount];
unique_ptr<CConstantBuffer> CEmitterManagerCommonData::GEmitterPositionBuffer = nullptr;
unique_ptr<CConstantBuffer> CEmitterManagerCommonData::GEmitterIndexBuffer = nullptr;

void CEmitterManagerCommonData::Intialize(
	UINT particleMaxCount, 
	ID3D11Device* device
)
{
#pragma region Particle 관련 멤버변수
	GParticleMaxCount = particleMaxCount;

	GTotalParticles = make_unique<CStructuredBuffer>(static_cast<UINT>(sizeof(SParticle)), GParticleMaxCount, nullptr);
	GDeathIndexSet = make_unique<CAppendBuffer>(4, GParticleMaxCount, nullptr);

	GTotalParticles->InitializeBuffer(device);
	GDeathIndexSet->InitializeBuffer(device);
#pragma endregion

#pragma region Particle Initialize 관련 CS
	GInitializeParticleSetCS->CreateShader(L"./InitializeParticleSetCS.hlsl", nullptr, "main", "cs_5_0", device);
#pragma endregion

#pragma region Particle 소싱 관련 CS
	D3D_SHADER_MACRO sourceMacro[] = {
		{ "INITIAL_SOURCE", nullptr },
		{ nullptr, nullptr }
	};
	GParticleInitialSourceCS->CreateShader(L"./ParticleSourceCS.hlsl", sourceMacro, "main", "cs_5_0", device);

	sourceMacro[0] = { "RUNTIME_SOURCE", nullptr };
	GParticleRuntimeSourceCS->CreateShader(L"./ParticleSourceCS.hlsl", sourceMacro, "main", "cs_5_0", device);
#pragma endregion

#pragma region Indirect 인자 계산 관련 CS
	GCalcualteIndirectArgCS->CreateShader(L"./ComputeIndirectArgsCS.hlsl", nullptr, "main", "cs_5_0", device);
#pragma endregion

#pragma region Particle 시뮬레이션 관련 CS
	GCaculateParticleForceCS->CreateShader(L"./CalculateParticleForceCS.hlsl", nullptr, "main", "cs_5_0", device);
#pragma endregion

#pragma region Particle 그리기 관련 PSO
	GParticleDrawVS->CreateShader(L"./ParticleDrawVS.hlsl", nullptr, "main", "vs_5_0", device);
	GParticleDrawGS->CreateShader(L"./ParticleDrawGS.hlsl", nullptr, "main", "gs_5_0", device);
	GParticleDrawPS->CreateShader(L"./ParticleDrawPS.hlsl", nullptr, "main", "ps_5_0", device);

	GDrawParticlePSO = make_unique<CGraphicsPSOObject>(
		GParticleDrawVS.get(),
		nullptr,
		nullptr,
		GParticleDrawGS.get(),
		GParticleDrawPS.get(),
		CRasterizerState::GetRSSolidCWSS(),
		CBlendState::GetBSAlphaBlend(),
		CDepthStencilState::GetDSDraw(),
		nullptr,
		0
	);
#pragma endregion

#pragma region Emitter 그리기 PSO
	GEmitterDrawVS = make_unique<CVertexShader>(5);

	for (size_t idx = 0; idx < EmitterTypeCount; ++idx)
	{
		GEmitterDrawPS[idx] = make_unique<CPixelShader>();
	}

	GEmitterPositionBuffer = make_unique<CConstantBuffer>(
		12, static_cast<UINT>(GEmitterBoxPositions.size()), GEmitterBoxPositions.data(), D3D11_BIND_VERTEX_BUFFER
		);
	GEmitterPositionBuffer->InitializeBuffer(device);

	GEmitterIndexBuffer = make_unique<CConstantBuffer>(
		4, static_cast<UINT>(GEmitterBoxIndices.size()), GEmitterBoxIndices.data(), D3D11_BIND_INDEX_BUFFER
		);
	GEmitterIndexBuffer->InitializeBuffer(device);

	GEmitterDrawVS->AddInputLayoutElement(
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	);
	GEmitterDrawVS->AddInputLayoutElement(
		{ "INSTANCE_WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	);
	GEmitterDrawVS->AddInputLayoutElement(
		{ "INSTANCE_WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	);
	GEmitterDrawVS->AddInputLayoutElement(
		{ "INSTANCE_WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	);
	GEmitterDrawVS->AddInputLayoutElement(
		{ "INSTANCE_WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
	);

	GEmitterDrawVS->CreateShader(L"./EmitterDrawVS.hlsl", nullptr, "main", "vs_5_0", device);

	D3D_SHADER_MACRO emitterDrawMacro[EmitterTypeCount][2] =
	{
		{
			{ "PARTICLE_EMITTER", nullptr },
			{ nullptr, nullptr }
		},
		{
			{ "RIBBON_EMITTER", nullptr },
			{ nullptr, nullptr }
		},
		{
			{ "SPRITE_EMITTER", nullptr },
			{ nullptr, nullptr }
		},
		{
			{ "MESH_EMITTER", nullptr },
			{ nullptr, nullptr }
		}
	};

	for (size_t idx = 0; idx < EmitterTypeCount; ++idx)
	{
		GEmitterDrawPS[idx]->CreateShader(L"./EmitterDrawPS.hlsl", emitterDrawMacro[idx], "main", "ps_5_0", device);
		GDrawEmitterPSO[idx] = make_unique<CGraphicsPSOObject>(
			GEmitterDrawVS.get(),
			nullptr,
			nullptr,
			nullptr,
			GEmitterDrawPS[idx].get(),
			CRasterizerState::GetRSWireframeCWSS(),
			nullptr,
			CDepthStencilState::GetDSDraw(),
			nullptr,
			0
		);
	}
#pragma endregion
}