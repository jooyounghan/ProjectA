#pragma once
#include <d3d11.h>
#include "EmitterForceProperty.h"

#include <memory>
#include <queue>

#define ZERO_MATRIX  DirectX::XMMATRIX(DirectX::XMVectorZero(), DirectX::XMVectorZero(), DirectX::XMVectorZero(), DirectX::XMVectorZero())

namespace D3D11
{
	class CDynamicBuffer;
	class CConstantBuffer;
	class CStructuredBuffer;

	class CComputeShader;
	class CVertexShader;
	class CGeometryShader;
	class CPixelShader;
	class CGraphicsPSOObject;
}

struct SEmitterInterpolaterInformation
{
	float maxLife;
	uint32_t colorInterpolaterID;
	uint32_t colorInterpolaterDegree;
	float particleDummy;
};

class EmitterStaticData
{
	friend class AEmitter;
	friend class CEmitterManager;
	friend class CEmitterSelector;

protected:
	static UINT GEmitterMaxCount;
	static std::queue<UINT> GEmitterIDQueue;

public:
	static UINT IssueAvailableEmitterID();
	static void ReclaimEmitterID(UINT emitterID) noexcept;

#pragma region EmitterWorldTransformation
protected:
	static std::vector<DirectX::XMMATRIX> GEmitterWorldTransformCPU;
	static std::vector<UINT> GChangedEmitterWorldTransformIDs;

public:
	static std::unique_ptr<D3D11::CDynamicBuffer> GEmitterInstancedTrnasformGPU;
	static std::unique_ptr<D3D11::CStructuredBuffer> GEmitterWorldTrnasformGPU;

public:
	static void AddChangedEmitterTransformID(UINT emitterID);
#pragma endregion

#pragma region Emitter Force
protected:
	static std::vector<SEmitterForceProperty> GEmitterForcePropertyCPU;
	static std::vector<UINT> GChangedEmitterForceIDs;

public:
	static std::unique_ptr<D3D11::CStructuredBuffer> GEmitterForcePropertyGPU;

public:
	static void AddChangedEmitterForceID(UINT emitterID);
#pragma endregion

#pragma region Emitter Interpolater Property
protected:
	static std::vector<SEmitterInterpolaterInformation> GEmitterInterpolaterInformationCPU;
	static std::vector<UINT> GChangedEmitterInterpolaterInformationIDs;

public:
	static std::unique_ptr<D3D11::CStructuredBuffer> GEmitterInterpolaterInformationGPU;

public:
	static void AddChangedEmitterInterpolaterInformationID(UINT emitterID);
#pragma endregion

public:
	static void InitializeGlobalEmitterProperty(UINT emitterMaxCount, ID3D11Device* device);
	static void UpdateGlobalEmitterProperty(ID3D11DeviceContext* deviceContext);


#pragma region Emitter ±×¸®±â PSO
public:
	static const std::vector<DirectX::XMFLOAT3> GEmitterBoxPositions;
	static const std::vector<UINT> GEmitterBoxIndices;
	static std::unique_ptr<D3D11::CVertexShader> GEmitterDrawVS;
	static std::unique_ptr<D3D11::CPixelShader> GEmitterDrawPS;
	static std::unique_ptr<D3D11::CGraphicsPSOObject> GDrawEmitterPSO;
	static std::unique_ptr<D3D11::CConstantBuffer> GEmitterPositionBuffer;
	static std::unique_ptr<D3D11::CConstantBuffer> GEmitterIndexBuffer;
	static void InitializeEmitterDrawPSO(ID3D11Device* device);
	static void DrawEmittersDebugCube(ID3D11DeviceContext* deviceContext);
#pragma endregion

};

