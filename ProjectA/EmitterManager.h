#pragma once
#include "Updatable.h"
#include "DispatchIndirectStructure.h"
#include "IndirectBuffer.h"
#include "AppendBuffer.h"

#include <vector>
#include <memory>

namespace D3D11
{
	class CStructuredBuffer;
	class CAppendBuffer;
}

class AEmitter;

class CEmitterManager : public IUpdatable
{
public:
	CEmitterManager(UINT emitterTypeCount, UINT particleMaxCount);
	~CEmitterManager() = default;

#pragma region Emitter 멤버 변수 / 함수
protected:
	std::vector<std::unique_ptr<AEmitter>> m_emitters;

public:
	void AddParticleEmitter(std::unique_ptr<AEmitter>& emitter, ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	inline std::vector<std::unique_ptr<AEmitter>>& GetEmitters() noexcept { return m_emitters; };
	AEmitter* GetEmitter(UINT emitterID);
	 void RemoveParticleEmitter(UINT emitterID);

private:
	bool FindEmitterFromID(UINT emitterID, OUT std::vector<std::unique_ptr<AEmitter>>::iterator& iter);
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

public:
	inline const UINT& GetParticleMaxCount() const noexcept { return m_particleMaxCount; };

protected:
	std::unique_ptr<D3D11::CStructuredBuffer> m_alivePrefixSums;
	std::unique_ptr<D3D11::CStructuredBuffer> m_alivePrefixDescriptors;
	std::unique_ptr<D3D11::CStructuredBuffer> m_indicesBuffers;

//protected:
//	struct
//	{
//		UINT passCount;
//		UINT bitOffset;
//		UINT dummy[2];
//	} m_sortStatusCPU;
//	std::unique_ptr<D3D11::CDynamicBuffer> m_sortStatusGPU;
//	std::unique_ptr<D3D11::CStructuredBuffer> m_countBuffers;
//	std::unique_ptr<D3D11::CStructuredBuffer> m_countPrefixDescriptors;
//	std::unique_ptr<D3D11::CStructuredBuffer> m_sortedIndicesBuffers;

//protected:
//	UINT m_emitterTypeCount;
//	std::unique_ptr<D3D11::CIndirectBuffer<D3D11_DISPATCH_INDIRECT_ARGS>> m_emitterPerTypeDispatchIndirectBuffer;

protected:
	std::unique_ptr<D3D11::CStructuredBuffer> m_particleDrawIndirectStagingGPU;
	std::unique_ptr<D3D11::CStructuredBuffer> m_IndexedParticleDispatchIndirectStagingGPU;
	std::unique_ptr<D3D11::CIndirectBuffer<D3D11_DRAW_INSTANCED_INDIRECT_ARGS>> m_particleDrawIndirectBuffer;
	std::unique_ptr<D3D11::CIndirectBuffer<D3D11_DISPATCH_INDIRECT_ARGS>> m_indexedParticleDispatchIndirectBuffer;

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

public:
	void ExecuteParticleSystem(ID3D11DeviceContext* deviceContext);
	void CaculateParticlesForce(ID3D11DeviceContext* deviceContext);
	void DrawParticles(ID3D11DeviceContext* deviceContext);

private:
	void InitializeParticleSet(ID3D11DeviceContext* deviceContext);
	void SourceEmitter(ID3D11DeviceContext* deviceContext);
	void PoolingParticles(ID3D11DeviceContext* deviceContext);
	void SortingParitcleIndices(ID3D11DeviceContext* deviceContext);
};