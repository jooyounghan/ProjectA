#include "EmitterManagerStaticData.h"

#include "ComputeShader.h"
#include "VertexShader.h"
#include "GeometryShader.h"
#include "PixelShader.h"
#include "GraphicsPSOObject.h"

#include "RasterizerState.h"
#include "BlendState.h"
#include "DepthStencilState.h"

using namespace std;
using namespace D3D11;

unique_ptr<CComputeShader> CEmitterManagerStaticData::GInitializeParticleSetCS = make_unique<CComputeShader>();

void CEmitterManagerStaticData::InitializeSetInitializingPSO(ID3D11Device* device)
{
	GInitializeParticleSetCS->CreateShader(L"./InitializeParticleSetCS.hlsl", nullptr, "main", "cs_5_0", device);
}

unique_ptr<CComputeShader> CEmitterManagerStaticData::GCalculateIndexPrefixSumCS = make_unique<CComputeShader>();
unique_ptr<CComputeShader> CEmitterManagerStaticData::GUpdateCurrentIndicesCS = make_unique<CComputeShader>();

void CEmitterManagerStaticData::InitializePoolingCS(ID3D11Device* device)
{
	GCalculateIndexPrefixSumCS->CreateShader(L"./CalculateIndexPrefixSumCS.hlsl", nullptr, "main", "cs_5_0", device);
	GUpdateCurrentIndicesCS->CreateShader(L"./UpdateCurrentIndicesCS.hlsl", nullptr, "main", "cs_5_0", device);
}

unique_ptr<CComputeShader> CEmitterManagerStaticData::GParticleInitialSourceCS = make_unique<CComputeShader>();
unique_ptr<CComputeShader> CEmitterManagerStaticData::GParticleRuntimeSourceCS = make_unique<CComputeShader>();

void CEmitterManagerStaticData::InitializeEmitterSourcingCS(ID3D11Device* device)
{
	D3D_SHADER_MACRO sourceMacro[] = {
		{ "INITIAL_SOURCE", nullptr },
		{ nullptr, nullptr }
	};
	GParticleInitialSourceCS->CreateShader(L"./ParticleSourceCS.hlsl", sourceMacro, "main", "cs_5_0", device);

	sourceMacro[0] = { "RUNTIME_SOURCE", nullptr };
	GParticleRuntimeSourceCS->CreateShader(L"./ParticleSourceCS.hlsl", sourceMacro, "main", "cs_5_0", device);
}

unique_ptr<CComputeShader> CEmitterManagerStaticData::GCaculateParticleForceCS = make_unique<CComputeShader>();

void CEmitterManagerStaticData::InitializeParticleSimulateCS(ID3D11Device* device)
{
	GCaculateParticleForceCS->CreateShader(L"./CalculateParticleForceCS.hlsl", nullptr, "main", "cs_5_0", device);
}

unique_ptr<CComputeShader> CEmitterManagerStaticData::GIndexRadixSortCS = make_unique<CComputeShader>();

void CEmitterManagerStaticData::InitializeRadixSortCS(ID3D11Device* device)
{
	GIndexRadixSortCS->CreateShader(L"./IndexRadixSortCS.hlsl", nullptr, "main", "cs_5_0", device);
}

unique_ptr<CVertexShader> CEmitterManagerStaticData::GParticleDrawVS = make_unique<CVertexShader>(0);
unique_ptr<CGeometryShader> CEmitterManagerStaticData::GParticleDrawGS = make_unique<CGeometryShader>();
unique_ptr<CPixelShader> CEmitterManagerStaticData::GParticleDrawPS = make_unique<CPixelShader>();
unique_ptr<CGraphicsPSOObject> CEmitterManagerStaticData::GDrawParticlePSO = nullptr;

void CEmitterManagerStaticData::InitializeParticleDrawPSO(ID3D11Device* device)
{
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
}