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


#pragma region Emitter ��� ���� / �Լ�
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

