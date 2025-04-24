#pragma once
#include <d3d11.h>
#include <memory>

namespace D3D11
{
	class CComputeShader;
	class CVertexShader;
	class CGeometryShader;
	class CPixelShader;
	class CGraphicsPSOObject;
	class CStructuredBuffer;
	class CAppendBuffer;
}

class CEmitterManagerStaticData
{
#pragma region Particle System ���� PSO
public:
#pragma region Particle Initialize ���� CS
	static std::unique_ptr<D3D11::CComputeShader> GInitializeParticleSetCS;
	static void InitializeSetInitializingPSO(ID3D11Device* device);
#pragma endregion

#pragma region Particle Ǯ�� ���� CS
public:
	static std::unique_ptr<D3D11::CComputeShader> GCalculateIndexPrefixSumCS;
	static std::unique_ptr<D3D11::CComputeShader> GUpdateCurrentIndicesCS;
	static void InitializePoolingCS(ID3D11Device* device);
#pragma endregion

#pragma region Particle �ҽ� ���� CS(���� Emitter ����� ���� Ȯ�� ����)
public:
	static std::unique_ptr<D3D11::CComputeShader> GParticleInitialSourceCS;
	static std::unique_ptr<D3D11::CComputeShader> GParticleRuntimeSourceCS;
	static void InitializeEmitterSourcingCS(ID3D11Device* device);
#pragma endregion

#pragma region Particle �ùķ��̼� ���� CS(���� Emitter ����� ���� Ȯ�� ����)
public:
	static std::unique_ptr<D3D11::CComputeShader> GCaculateParticleForceCS;
	static void InitializeParticleSimulateCS(ID3D11Device* device);
#pragma endregion

#pragma region Index Sorting(Radix Sorting) ���� CS
	static std::unique_ptr<D3D11::CComputeShader> GIndexRadixSortCS;
	static void InitializeRadixSortCS(ID3D11Device* device);

#pragma endregion

#pragma region Particle �׸��� ���� PSO
public:
	static std::unique_ptr<D3D11::CVertexShader> GParticleDrawVS;
	static std::unique_ptr<D3D11::CGeometryShader> GParticleDrawGS;
	static std::unique_ptr<D3D11::CPixelShader> GParticleDrawPS;
	static std::unique_ptr<D3D11::CGraphicsPSOObject> GDrawParticlePSO;
	static void InitializeParticleDrawPSO(ID3D11Device* device);
#pragma endregion

#pragma endregion
};

