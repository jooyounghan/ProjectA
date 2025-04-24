#include "EmitterStaticData.h"

#include "MacroUtilities.h"

#include "ComputeShader.h"
#include "VertexShader.h"
#include "GeometryShader.h"
#include "PixelShader.h"
#include "GraphicsPSOObject.h"

#include "RasterizerState.h"
#include "BlendState.h"
#include "DepthStencilState.h"

#include "ConstantBuffer.h"
#include "StructuredBuffer.h"

#include "ModelFactory.h"

using namespace std;
using namespace DirectX;
using namespace D3D11;

UINT EmitterStaticData::GEmitterMaxCount = 0;
queue<UINT> EmitterStaticData::GEmitterIDQueue;

vector<XMMATRIX> EmitterStaticData::GEmitterWorldTransformCPU;
unique_ptr<CDynamicBuffer> EmitterStaticData::GEmitterWorldTransformGPU = nullptr;
vector<UINT> EmitterStaticData::GChangedEmitterWorldPositionIDs;

vector<SEmitterForceProperty> EmitterStaticData::GEmitterForcePropertyCPU;
unique_ptr<CStructuredBuffer> EmitterStaticData::GEmitterForcePropertyGPU = nullptr;
vector<UINT> EmitterStaticData::GChangedEmitterForceIDs;

vector<SEmitterInterpolaterInformation> EmitterStaticData::GEmitterInterpolaterInformationCPU;
unique_ptr<CStructuredBuffer> EmitterStaticData::GEmitterInterpolaterInformationGPU = nullptr;
vector<UINT> EmitterStaticData::GChangedEmitterInterpolaterInformationIDs;

const vector<XMFLOAT3> EmitterStaticData::GEmitterBoxPositions = ModelFactory::CreateBoxPositions(XMVectorSet(1.f, 1.f, 1.f, 0.f));
const vector<UINT> EmitterStaticData::GEmitterBoxIndices = ModelFactory::CreateIndices();
unique_ptr<CVertexShader> EmitterStaticData::GEmitterDrawVS = make_unique<CVertexShader>(5);
unique_ptr<CPixelShader> EmitterStaticData::GEmitterDrawPS = make_unique<CPixelShader>();
unique_ptr<CGraphicsPSOObject> EmitterStaticData::GDrawEmitterPSO = nullptr;
unique_ptr<CConstantBuffer> EmitterStaticData::GEmitterPositionBuffer = nullptr;
unique_ptr<CConstantBuffer> EmitterStaticData::GEmitterIndexBuffer = nullptr;


UINT EmitterStaticData::IssueAvailableEmitterID()
{
	if (GEmitterIDQueue.empty()) { throw exception("No Emitter ID To Issue"); }

	UINT emitterID = GEmitterIDQueue.front();
	GEmitterIDQueue.pop();

	return emitterID;
}

void EmitterStaticData::ReclaimEmitterID(UINT emitterID) noexcept
{
	GEmitterIDQueue.push(emitterID);
}

void EmitterStaticData::AddChangedEmitterWorldPositionID(UINT emitterID)
{
	GChangedEmitterWorldPositionIDs.emplace_back(emitterID);
}

void EmitterStaticData::AddChangedEmitterForceID(UINT emitterID)
{
	GChangedEmitterForceIDs.emplace_back(emitterID);
}

void EmitterStaticData::AddChangedEmitterInterpolaterInformationID(UINT emitterID)
{
	GChangedEmitterInterpolaterInformationIDs.emplace_back(emitterID);
}

void EmitterStaticData::InitializeGlobalEmitterProperty(UINT emitterMaxCount, ID3D11Device* device)
{
	GEmitterMaxCount = emitterMaxCount;
	for (UINT idx = 0; idx < GEmitterMaxCount; ++idx)
	{
		GEmitterIDQueue.push(idx);
	}

	SEmitterForceProperty initialForceProperty;
	AutoZeroMemory(initialForceProperty);

	SEmitterInterpolaterInformation initialEmitterInterpolationInformation;
	AutoZeroMemory(initialEmitterInterpolationInformation);

	GEmitterWorldTransformCPU.resize(GEmitterMaxCount, ZERO_MATRIX);
	GEmitterForcePropertyCPU.resize(GEmitterMaxCount, initialForceProperty);
	GEmitterInterpolaterInformationCPU.resize(GEmitterMaxCount, initialEmitterInterpolationInformation);

	GEmitterWorldTransformGPU = make_unique<CDynamicBuffer>(
		static_cast<UINT>(sizeof(XMMATRIX)),
		GEmitterMaxCount,
		GEmitterWorldTransformCPU.data(),
		D3D11_BIND_VERTEX_BUFFER
	);
	GEmitterForcePropertyGPU = make_unique<CStructuredBuffer>(
		static_cast<UINT>(sizeof(SEmitterForceProperty)),
		GEmitterMaxCount,
		GEmitterForcePropertyCPU.data()
	);
	GEmitterInterpolaterInformationGPU = make_unique<CStructuredBuffer>(
		static_cast<UINT>(sizeof(SEmitterInterpolaterInformation)),
		GEmitterMaxCount,
		GEmitterInterpolaterInformationCPU.data()
	);

	GEmitterWorldTransformGPU->InitializeBuffer(device);
	GEmitterForcePropertyGPU->InitializeBuffer(device);
	GEmitterInterpolaterInformationGPU->InitializeBuffer(device);
}

void EmitterStaticData::UpdateGlobalEmitterProperty(ID3D11DeviceContext* deviceContext)
{
	UINT emitterWorldPositionChangedIDsCount = static_cast<UINT>(GChangedEmitterWorldPositionIDs.size());
	if (emitterWorldPositionChangedIDsCount > 0)
	{
		GEmitterWorldTransformGPU->StageNthElement(deviceContext, GChangedEmitterWorldPositionIDs.data(), emitterWorldPositionChangedIDsCount);
		GEmitterWorldTransformGPU->UploadNthElement(deviceContext, GChangedEmitterWorldPositionIDs.data(), emitterWorldPositionChangedIDsCount);
		GChangedEmitterWorldPositionIDs.clear();
	}

	UINT emitterForceChagnedIDsCount = static_cast<UINT>(GChangedEmitterForceIDs.size());
	if (emitterForceChagnedIDsCount > 0)
	{
		GEmitterForcePropertyGPU->StageNthElement(deviceContext, GChangedEmitterForceIDs.data(), emitterForceChagnedIDsCount);
		GEmitterForcePropertyGPU->UploadNthElement(deviceContext, GChangedEmitterForceIDs.data(), emitterForceChagnedIDsCount);
		GChangedEmitterForceIDs.clear();
	}

	UINT emitterInterpolaterInformationChagnedIDsCount = static_cast<UINT>(GChangedEmitterInterpolaterInformationIDs.size());
	if (emitterInterpolaterInformationChagnedIDsCount > 0)
	{
		GEmitterInterpolaterInformationGPU->StageNthElement(deviceContext, GChangedEmitterInterpolaterInformationIDs.data(), emitterInterpolaterInformationChagnedIDsCount);
		GEmitterInterpolaterInformationGPU->UploadNthElement(deviceContext, GChangedEmitterInterpolaterInformationIDs.data(), emitterInterpolaterInformationChagnedIDsCount);
		GChangedEmitterInterpolaterInformationIDs.clear();
	}
}

void EmitterStaticData::InitializeEmitterDrawPSO(ID3D11Device* device)
{
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
	GEmitterDrawPS->CreateShader(L"./EmitterDrawPS.hlsl", nullptr, "main", "ps_5_0", device);

	GDrawEmitterPSO = make_unique<CGraphicsPSOObject>(
		GEmitterDrawVS.get(),
		nullptr,
		nullptr,
		nullptr,
		GEmitterDrawPS.get(),
		CRasterizerState::GetRSWireframeCWSS(),
		nullptr,
		CDepthStencilState::GetDSDraw(),
		nullptr,
		0
	);
}

void EmitterStaticData::DrawEmittersDebugCube(ID3D11DeviceContext* deviceContext)
{
	static vector<ID3D11Buffer*> vertexBuffer = { GEmitterPositionBuffer->GetBuffer(), GEmitterWorldTransformGPU->GetBuffer() };
	static vector<ID3D11Buffer*> vertexNullBuffer = vector<ID3D11Buffer*>(vertexBuffer.size(), nullptr);
	static ID3D11Buffer* indexBuffer = GEmitterIndexBuffer->GetBuffer();
	static vector<UINT> strides = { static_cast<UINT>(sizeof(XMFLOAT3)), static_cast<UINT>(sizeof(XMMATRIX)) };
	static vector<UINT> nullStrides = vector<UINT>(strides.size(), NULL);
	static vector<UINT> offsets = { 0, 0 };
	static vector<UINT> nullOffsets = vector<UINT>(nullStrides.size(), NULL);

	GDrawEmitterPSO->ApplyPSO(deviceContext);

	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	deviceContext->IASetVertexBuffers(0, static_cast<UINT>(vertexBuffer.size()), vertexBuffer.data(), strides.data(), offsets.data());
	deviceContext->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R32_UINT, NULL);
	deviceContext->DrawIndexedInstanced(
		static_cast<UINT>(GEmitterBoxIndices.size()),
		GEmitterMaxCount,
		NULL, NULL, NULL
	);
	deviceContext->IASetVertexBuffers(0, static_cast<UINT>(vertexNullBuffer.size()), vertexNullBuffer.data(), nullStrides.data(), nullOffsets.data());
	deviceContext->IASetIndexBuffer(nullptr, DXGI_FORMAT_R32_UINT, NULL);

	GDrawEmitterPSO->RemovePSO(deviceContext);
}