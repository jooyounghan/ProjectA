#pragma once
#include "ParticleEmitter.h"

#include "StructuredBuffer.h"
#include "AppendBuffer.h"
#include "IndirectBuffer.h"

#include <vector>
#include <queue>

namespace D3D11
{
	class CConstantBuffer;
	class CDynamicBuffer;

	class CComputeShader;
	class CVertexShader;
	class CGeometryShader;
	class CPixelShader;

	class CGraphicsPSOObject;
}

class CParticleManager : public IUpdatable
{
public:
	CParticleManager(UINT emitterMaxCount, UINT particleMaxCount);
	~CParticleManager() = default;

#pragma region Emitter 멤버 변수 / 함수
protected:
	UINT m_emitterMaxCount;
	std::queue<UINT> m_transformIndexQueue;
	std::vector<std::unique_ptr<CParticleEmitter>> m_particleEmitters;
	std::vector<DirectX::XMMATRIX> m_emitterWorldTransformCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_emitterWorldTransformGPU;
	std::vector<DirectX::XMVECTOR> m_emitterWolrdPosCPU;
	std::unique_ptr<D3D11::CStructuredBuffer> m_emitterWorldPosGPU;
	bool m_isEmitterWorldPositionChanged;
	bool m_isEmitterWorldTransformationChanged;

public:
	UINT AddParticleEmitter(
		UINT emitterType,
		float paritlceDensity,
		float particleRadius,
		const DirectX::XMVECTOR& position,
		const DirectX::XMVECTOR& angle,
		const DirectX::XMFLOAT2& minInitRadians,
		const DirectX::XMFLOAT2& maxInitRadians,
		const DirectX::XMFLOAT2& minMaxRadius,
		UINT initialParticleCount,
		ID3D11Device* device, 
		ID3D11DeviceContext* deviceContext

	);
	CParticleEmitter* GetEmitter(UINT emitterID);
	 void RemoveParticleEmitter(UINT emitterID);

private:
	bool FindEmitterFromID(UINT emitterID, OUT std::vector<std::unique_ptr<CParticleEmitter>>::iterator& iter);

public:
	 inline const std::vector<std::unique_ptr<CParticleEmitter>>& GetParticleEmitters() const noexcept { return m_particleEmitters; }
#pragma endregion

#pragma region Particle System 관련 PSO
#pragma region Emitter 그리기 PSO
public:
	static const std::vector<DirectX::XMFLOAT3> GEmitterBoxPositions;
	static const std::vector<UINT> GEmitterBoxIndices;
	static std::unique_ptr<D3D11::CVertexShader> GEmitterDrawVS;
	static std::unique_ptr<D3D11::CPixelShader> GEmitterDrawPS;
	static std::unique_ptr<D3D11::CGraphicsPSOObject> GDrawEmitterPSO;
	static std::unique_ptr<D3D11::CConstantBuffer> GEmitterPositionBuffer;
	static std::unique_ptr<D3D11::CConstantBuffer> GEmitterIndexBuffer;
	static void InitializeEmitterDrawPSO(ID3D11Device* device);
#pragma endregion

#pragma region Particle Initialize 관련 CS
	static std::unique_ptr<D3D11::CComputeShader> GInitializeParticleSetCS;
	static void InitializeSetInitializingPSO(ID3D11Device* device);
#pragma endregion

#pragma region Particle 풀링 관련 CS
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
	static std::unique_ptr<D3D11::CComputeShader> GCountIndexCS;
	static std::unique_ptr<D3D11::CComputeShader> GCaluclateCountPrefixSumCS;
	static std::unique_ptr<D3D11::CComputeShader> GReorderCS;
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

protected:
	struct SPrefixDesciptor
	{
		UINT aggregate;
		UINT statusFlag;
		UINT exclusivePrefix;
		UINT inclusivePrefix;
	};

protected:
	UINT m_particleMaxCount;
	std::unique_ptr<D3D11::CStructuredBuffer> m_totalParticles;
	std::unique_ptr<D3D11::CStructuredBuffer> m_aliveFlags;
	std::unique_ptr<D3D11::CAppendBuffer> m_deathParticleSet;

protected:
	std::unique_ptr<D3D11::CStructuredBuffer> m_alivePrefixSums;
	std::unique_ptr<D3D11::CStructuredBuffer> m_alivePrefixDescriptors;
	std::unique_ptr<D3D11::CStructuredBuffer> m_indicesBuffers;

protected:
	std::unique_ptr<D3D11::CStructuredBuffer> m_countBuffers;
	std::unique_ptr<D3D11::CStructuredBuffer> m_countPrefixSums;
	std::unique_ptr<D3D11::CStructuredBuffer> m_countPrefixDescriptors;
	std::unique_ptr<D3D11::CStructuredBuffer> m_sortedIndicesBuffers;

public:
	inline const UINT& GetParticleMaxCount() const noexcept { return m_particleMaxCount; };

protected:
	std::unique_ptr<D3D11::CStructuredBuffer> m_particleDrawIndirectStagingGPU;
	std::unique_ptr<D3D11::CStructuredBuffer> m_IndexedParticleDispatchIndirectStagingGPU;
	std::unique_ptr<D3D11::CIndirectBuffer<D3D11_DRAW_INSTANCED_INDIRECT_ARGS>> m_particleDrawIndirectBuffer;
	std::unique_ptr<D3D11::CIndirectBuffer<D3D11_DISPATCH_INDIRECT_ARGS>> m_indexedParticleDispatchIndirectBuffer;

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

public:
	void DrawEmittersDebugCube(ID3D11DeviceContext* deviceContext);
	void ExecuteParticleSystem(ID3D11DeviceContext* deviceContext);
	void DrawParticles(ID3D11DeviceContext* deviceContext);

private:
	void InitializeParticleSet(ID3D11DeviceContext* deviceContext);
	void SourceEmitter(ID3D11DeviceContext* deviceContext);
	void PoolingParticles(ID3D11DeviceContext* deviceContext);
	void CaculateParticlesForce(ID3D11DeviceContext* deviceContext);
};