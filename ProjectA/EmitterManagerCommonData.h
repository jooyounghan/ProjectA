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
	static void Intialize(UINT particleMaxCount, ID3D11Device* device);

#pragma region Particle ���� �������
public:
	static UINT GParticleMaxCount;
	static std::unique_ptr<D3D11::CStructuredBuffer> GTotalParticles;
	static std::unique_ptr<D3D11::CAppendBuffer> GDeathIndexSet;
#pragma endregion

#pragma region Particle Initialize ���� CS
public:
	static std::unique_ptr<D3D11::CComputeShader> GInitializeParticleSetCS;
#pragma endregion

#pragma region Particle �ҽ� ���� CS
public:
	static std::unique_ptr<D3D11::CComputeShader> GParticleInitialSourceCS;
	static std::unique_ptr<D3D11::CComputeShader> GParticleRuntimeSourceCS;
#pragma endregion

#pragma region Indirect ���� ��� ���� CS
public:
	static std::unique_ptr<D3D11::CComputeShader> GCalcualteIndirectArgCS;
#pragma endregion


#pragma region Particle �ùķ��̼� ���� CS
public:
	static std::unique_ptr<D3D11::CComputeShader> GCaculateParticleForceCS;
#pragma endregion

#pragma region Particle �׸��� ���� PSO
public:
	static std::unique_ptr<D3D11::CVertexShader> GParticleDrawVS;
	static std::unique_ptr<D3D11::CGeometryShader> GParticleDrawGS;
	static std::unique_ptr<D3D11::CPixelShader> GParticleDrawPS;
	static std::unique_ptr<D3D11::CGraphicsPSOObject> GDrawParticlePSO;
#pragma endregion

#pragma region Emitter �׸��� PSO
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



