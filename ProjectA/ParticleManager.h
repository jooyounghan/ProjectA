#pragma once
#include "ParticleEmitter.h"
#include "StructuredBuffer.h"
#include "AppendBuffer.h"

#include <vector>
#include <queue>

namespace D3D11
{
	class CConstantBuffer;
	class CDynamicBuffer;
	class CStructuredBuffer;
	class CAppendBuffer;

	class CVertexShader;
	class CPixelShader;
	class CGraphicsPSOObject;
}

class CParticleManager : public IUpdatable
{
public:
	CParticleManager(UINT maxEmitterCount);
	~CParticleManager() = default;


#pragma region Emitter 멤버 변수 / 함수
public:
	static const std::vector<DirectX::XMFLOAT3> GEmitterBoxPositions;
	static const std::vector<UINT> GEmitterBoxIndices;
	static std::unique_ptr<D3D11::CVertexShader> GEmitterDrawVS;
	static std::unique_ptr<D3D11::CPixelShader> GEmitterDrawPS;
	static std::unique_ptr<D3D11::CGraphicsPSOObject> GDrawEmitterPSO;
	static std::unique_ptr<D3D11::CConstantBuffer> GEmitterPositionBuffer;
	static std::unique_ptr<D3D11::CConstantBuffer> GEmitterIndexBuffer;
	static void InitializeEmitterDrawPSO(ID3D11Device* device);

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
protected:
	struct
	{
		float particleCountCurr;
		float dummy[3];
	} m_particleManagerPropteriesCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_particleManagerPropteriesGPU;

protected:
	std::unique_ptr<D3D11::CStructuredBuffer> m_totalParticlePool;
	std::unique_ptr<D3D11::CAppendBuffer> m_deathParticlePool;

public:
	inline ID3D11ShaderResourceView* GetTotalParticlePoolSRV() const noexcept { return m_totalParticlePool->GetSRV(); }
	inline ID3D11UnorderedAccessView* GetTotalParticlePoolUAV() const noexcept { return m_totalParticlePool->GetUAV(); }
	inline ID3D11ShaderResourceView* GetDeathParitclePoolSRV() const noexcept { m_deathParticlePool->GetSRV(); }
	inline ID3D11UnorderedAccessView* GetDeathParitclePoolUAV() const noexcept { m_deathParticlePool->GetUAV(); }

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

public:
	void DrawEmittersDebugCube(ID3D11Buffer* viewProjBuffer, ID3D11DeviceContext* deviceContext);
	void PresetParticleSet();
	void ActivateEmitter();
	void DeframentPool();
	void SimulateParticles();
	void SortParticles();
	void DrawParticles();

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

