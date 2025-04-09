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
	bool m_isEmitterWorldTransformationChanged;

public:
	UINT AddParticleEmitter(
		const DirectX::XMVECTOR& position,
		const DirectX::XMVECTOR& angle,
		const DirectX::XMVECTOR& emitVelocity,
		const std::vector<SEmitTimeRate>& emitProfiles,
		UINT emitterType,
		ID3D11Device* device, 
		ID3D11DeviceContext* deviceContext

	);
	 void RemoveParticleEmitter(UINT emitterID);

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

#pragma region Particle 풀링 관련 PSO
	static std::unique_ptr<D3D11::CComputeShader> GSelectParticleSetCS;
	static std::unique_ptr<D3D11::CComputeShader> GCalculatePrefixSumCS;
	static std::unique_ptr<D3D11::CComputeShader> GUpdateCurrentIndicesCS;
	//static std::unique_ptr<D3D11::CComputeShader> GDefragmenaPoolCS;
	static void InitializePoolingPSO(ID3D11Device* device);
#pragma endregion

#pragma region Particle 소싱 관련 PSO(추후 Emitter 상속을 통한 확장 설계)
public:
	static std::unique_ptr<D3D11::CComputeShader> GParticleSourcingCS;
	static void InitializeEmitterSourcingPSO(ID3D11Device* device);
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
	struct SPartitionDescriptor
	{
		int aggregate;
		UINT statusFlag;
		int exclusivePrefix;
		int inclusivePrefix;
	};

protected:
	UINT m_particleMaxCount;
	std::unique_ptr<D3D11::CStructuredBuffer> m_totalParticles;
	std::unique_ptr<D3D11::CStructuredBuffer> m_aliveFlags;
	std::unique_ptr<D3D11::CStructuredBuffer> m_prefixSums;
	std::unique_ptr<D3D11::CStructuredBuffer> m_partitionDescriptors;
	std::unique_ptr<D3D11::CStructuredBuffer> m_indicesBuffers;
	std::unique_ptr<D3D11::CAppendBuffer> m_deathParticleSet;

public:
	inline const UINT& GetParticleMaxCount() const noexcept { return m_particleMaxCount; };

protected:
	std::unique_ptr<D3D11::CStructuredBuffer> m_particleDrawIndirectStagingGPU;
	std::unique_ptr<D3D11::CIndirectBuffer<D3D11_DRAW_INSTANCED_INDIRECT_ARGS>> m_particleDrawIndirectBuffer;

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

public:
	void DrawEmittersDebugCube(ID3D11Buffer* viewProjBuffer, ID3D11DeviceContext* deviceContext);
	void ExecuteParticleSystem(ID3D11DeviceContext* deviceContext);
	void DrawParticles(ID3D11DeviceContext* deviceContext);

private:
	void SelectParticleSet(ID3D11DeviceContext* deviceContext);
	void SourceEmitter(ID3D11DeviceContext* deviceContext);
	void CalculatePrefixSum(ID3D11DeviceContext* deviceContext);
	void GetCurrentIndices(ID3D11DeviceContext* deviceContext);
	void SortParticles(ID3D11DeviceContext* deviceContext);
	void SimulateParticles(ID3D11DeviceContext* deviceContext);
};