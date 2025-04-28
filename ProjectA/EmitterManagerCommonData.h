#pragma once
#include "EmitterTypeDefinition.h"
#include "DispatchIndirectStructure.h"

#include "IndirectBuffer.h"
#include "AppendBuffer.h"

#include <d3d11.h>
#include <DirectXMath.h>
#include <memory>
#include <vector>

namespace D3D11
{
	class CComputeShader;
	class CVertexShader;
	class CGeometryShader;
	class CPixelShader;
	class CGraphicsPSOObject;

	class CConstantBuffer;
	class CStructuredBuffer;
	class CAppendBuffer;
}

class CEmitterManagerCommonData
{
public:
	static void Intialize(ID3D11Device* device);

#pragma region Particle Initialize 관련 CS
public:
	static std::unique_ptr<D3D11::CComputeShader> GInitializeParticleSetCS[EmitterTypeCount];
#pragma endregion

#pragma region Particle 소싱 관련 CS
public:
	static std::unique_ptr<D3D11::CComputeShader> GParticleInitialSourceCS;
	static std::unique_ptr<D3D11::CComputeShader> GParticleRuntimeSourceCS;
#pragma endregion

#pragma region Indirect 인자 계산 관련 CS
public:
	static std::unique_ptr<D3D11::CComputeShader> GCalcualteIndirectArgCS;
#pragma endregion


#pragma region Particle 시뮬레이션 관련 CS
public:
	static std::unique_ptr<D3D11::CComputeShader> GCaculateParticleForceCS;
#pragma endregion

#pragma region Particle 그리기 관련 PSO
public:
	static std::unique_ptr<D3D11::CVertexShader> GParticleDrawVS;
	static std::unique_ptr<D3D11::CGeometryShader> GParticleDrawGS;
	static std::unique_ptr<D3D11::CPixelShader> GParticleDrawPS[EmitterTypeCount];
	static std::unique_ptr<D3D11::CGraphicsPSOObject> GDrawParticlePSO[EmitterTypeCount];
#pragma endregion

#pragma region Emitter 그리기 PSO
public:
	static const std::vector<DirectX::XMFLOAT3> GEmitterBoxPositions;
	static const std::vector<UINT> GEmitterBoxIndices;
	static std::unique_ptr<D3D11::CConstantBuffer> GEmitterPositionBuffer;
	static std::unique_ptr<D3D11::CConstantBuffer> GEmitterIndexBuffer;
	static std::unique_ptr<D3D11::CVertexShader> GEmitterDrawVS;
	static std::unique_ptr<D3D11::CPixelShader> GEmitterDrawPS[EmitterTypeCount];
	static std::unique_ptr<D3D11::CGraphicsPSOObject> GDrawEmitterPSO[EmitterTypeCount];
#pragma endregion
};



