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
	CParticleManager(UINT maxEmitterCount, UINT maxParticleCount);
	~CParticleManager() = default;

#pragma region Emitter 멤버 변수 / 함수
protected:
	UINT m_maxEmitterCount;
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
		ID3D11Device* device, ID3D11DeviceContext* deviceContext

	);
	 void RemoveParticleEmitter(UINT emitterID);
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
	static std::unique_ptr<D3D11::CComputeShader> GDefragmenaPoolCS;
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
	std::unique_ptr<D3D11::CStructuredBuffer> m_particleCountsGPU;

protected:
	std::unique_ptr<D3D11::CIndirectBuffer<D3D11_DISPATCH_INDIRECT_ARGS>> m_particleDispatchBuffer;

protected:
	UINT m_maxParticleCount;
	std::unique_ptr<D3D11::CStructuredBuffer> m_totalParticlePool;
	std::unique_ptr<D3D11::CAppendBuffer> m_deathParticleSet;

public:
	inline ID3D11ShaderResourceView* GetTotalParticlePoolSRV() const noexcept { return m_totalParticlePool->GetSRV(); }
	inline ID3D11UnorderedAccessView* GetTotalParticlePoolUAV() const noexcept { return m_totalParticlePool->GetUAV(); }
	inline ID3D11ShaderResourceView* GetDeathParitcleSetSRV() const noexcept { m_deathParticleSet->GetSRV(); }
	inline ID3D11UnorderedAccessView* GetDeathParitcleSetUAV() const noexcept { m_deathParticleSet->GetUAV(); }

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

public:
	void DrawEmittersDebugCube(ID3D11Buffer* viewProjBuffer, ID3D11DeviceContext* deviceContext);
	void ExecuteParticleSystem(ID3D11DeviceContext* deviceContext);
	void DrawParticles(ID3D11DeviceContext* deviceContext);

private:
	void PresetParticleSet(ID3D11DeviceContext* deviceContext);
	void ActivateEmitter(ID3D11DeviceContext* deviceContext);
	void DeframentPool(ID3D11DeviceContext* deviceContext);
	void SimulateParticles(ID3D11DeviceContext* deviceContext);
	void SortParticles(ID3D11DeviceContext* deviceContext);

	// 전체적인 그림이 먼저 필요함
	// A개의 Emiiter에서
	// B개 종류의 파티클이 생성됨 (A = N_1(1번 종류) + N_2(2번 종류) + ... + N_B(B번 종류))
	// 따라서 B개의 Sourcing, Simulating, Drawing 과정을 수행
	// Emitter 별로 종류 B에 대한 세이더의 포인터 들을 들고 있거나
	// 매니저가 전체적으로 수행...
	// 
	// 1. for (auto& emitter : m_emitters) emitter->SourceParticles(m_totalParticlePool, m_deathParticlePool); + 다형성(SparkEmitter, FireEmitter...)
	// 와 같은 식으로 하거나 m_sourceCS->Dispatch()로 한번에 하고 내부에서 플래그를 보고 수행
	// 
	// 시뮬레이션에 사용될 Field는 어떻게 처리할 지 생각 필요..
};

