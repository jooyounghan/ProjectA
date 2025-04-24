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
#pragma region Particle System 관련 PSO
public:
#pragma region Particle Initialize 관련 CS
	static std::unique_ptr<D3D11::CComputeShader> GInitializeParticleSetCS;
	static void InitializeSetInitializingPSO(ID3D11Device* device);
#pragma endregion

#pragma region Particle 풀링 관련 CS
public:
	static std::unique_ptr<D3D11::CComputeShader> GCalculateIndexPrefixSumCS;
	static std::unique_ptr<D3D11::CComputeShader> GUpdateCurrentIndicesCS;
	static void InitializePoolingCS(ID3D11Device* device);
#pragma endregion

#pragma region Particle 소싱 관련 CS(추후 Emitter 상속을 통한 확장 설계)
public:
	static std::unique_ptr<D3D11::CComputeShader> GParticleInitialSourceCS;
	static std::unique_ptr<D3D11::CComputeShader> GParticleRuntimeSourceCS;
	static void InitializeEmitterSourcingCS(ID3D11Device* device);
#pragma endregion

#pragma region Particle 시뮬레이션 관련 CS(추후 Emitter 상속을 통한 확장 설계)
public:
	static std::unique_ptr<D3D11::CComputeShader> GCaculateParticleForceCS;
	static void InitializeParticleSimulateCS(ID3D11Device* device);
#pragma endregion

#pragma region Index Sorting(Radix Sorting) 관련 CS
	static std::unique_ptr<D3D11::CComputeShader> GIndexRadixSortCS;
	static void InitializeRadixSortCS(ID3D11Device* device);

#pragma endregion

#pragma region Particle 그리기 관련 PSO
public:
	static std::unique_ptr<D3D11::CVertexShader> GParticleDrawVS;
	static std::unique_ptr<D3D11::CGeometryShader> GParticleDrawGS;
	static std::unique_ptr<D3D11::CPixelShader> GParticleDrawPS;
	static std::unique_ptr<D3D11::CGraphicsPSOObject> GDrawParticlePSO;
	static void InitializeParticleDrawPSO(ID3D11Device* device);
#pragma endregion

#pragma endregion
};

