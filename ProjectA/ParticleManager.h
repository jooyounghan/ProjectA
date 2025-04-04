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

#pragma region Emitter ��� ���� / �Լ�
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

#pragma region Particle System ���� PSO
#pragma region Emitter �׸��� PSO
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

#pragma region Particle Ǯ�� ���� PSO
	static std::unique_ptr<D3D11::CComputeShader> GSelectParticleSetCS;
	static std::unique_ptr<D3D11::CComputeShader> GDefragmenaPoolCS;
	static void InitializePoolingPSO(ID3D11Device* device);
#pragma endregion

#pragma region Particle �ҽ� ���� PSO(���� Emitter ����� ���� Ȯ�� ����)
public:
	static std::unique_ptr<D3D11::CComputeShader> GParticleSourcingCS;
	static void InitializeEmitterSourcingPSO(ID3D11Device* device);
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

	// ��ü���� �׸��� ���� �ʿ���
	// A���� Emiiter����
	// B�� ������ ��ƼŬ�� ������ (A = N_1(1�� ����) + N_2(2�� ����) + ... + N_B(B�� ����))
	// ���� B���� Sourcing, Simulating, Drawing ������ ����
	// Emitter ���� ���� B�� ���� ���̴��� ������ ���� ��� �ְų�
	// �Ŵ����� ��ü������ ����...
	// 
	// 1. for (auto& emitter : m_emitters) emitter->SourceParticles(m_totalParticlePool, m_deathParticlePool); + ������(SparkEmitter, FireEmitter...)
	// �� ���� ������ �ϰų� m_sourceCS->Dispatch()�� �ѹ��� �ϰ� ���ο��� �÷��׸� ���� ����
	// 
	// �ùķ��̼ǿ� ���� Field�� ��� ó���� �� ���� �ʿ�..
};

