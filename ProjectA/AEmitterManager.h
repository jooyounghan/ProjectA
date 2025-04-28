#pragma once
#include "IUpdatable.h"

#include "EmitterForceProperty.h"
#include "EmitterTypeDefinition.h"
#include "DispatchIndirectStructure.h"

#include "InterpInformation.h"
#include "InterpolaterStructure.h"
#include "Interpolater.h"

#include "IndirectBuffer.h"
#include "AppendBuffer.h"

#include <DirectXMath.h>
#include <vector>
#include <queue>
#include <memory>
#include <string>

class AEmitter;

template<uint32_t Dim, uint32_t CoefficientCount>
class CGPUInterpPropertyManager;

#define MaxParticleCount 1024 * 1024

class AEmitterManager : public IUpdatable
{
public:
	AEmitterManager(
		const std::string& managerName, 
		UINT emitterType,
		UINT maxEmitterCount,
		UINT maxParticleCount
	);
	~AEmitterManager() override;

protected:
	std::string m_managerName;

protected:
	void LoadManager(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	virtual UINT GetEmitterType() const noexcept = 0;

protected:
	struct alignas(16)
	{
		const UINT particleMaxCount;
		const UINT emitterType;
		const UINT padding1;
		const UINT padding2;
	} m_emitterManagerPropertyCPU;
	std::unique_ptr<D3D11::CDynamicBuffer> m_emitterManagerPropertyGPU;


protected:
	UINT m_maxEmitterCount;
	std::vector<std::unique_ptr<AEmitter>> m_emitters;
	std::queue<UINT> m_emitterIDQueue;

protected:
	UINT IssueAvailableEmitterID();
	virtual void ReclaimEmitterID(UINT emitterID) noexcept;

protected:
	std::unique_ptr<D3D11::CStructuredBuffer> m_totalParticles;
	std::unique_ptr<D3D11::CAppendBuffer> m_deathIndexSet;

protected:
	std::vector<DirectX::XMMATRIX> m_worldTransformCPU;
	std::vector<UINT> m_worldTransformChangedEmitterIDs;
	std::unique_ptr<D3D11::CDynamicBuffer> m_instancedWorldTransformGPU;
	std::unique_ptr<D3D11::CStructuredBuffer> m_worldTransformGPU;

public:
	void AddWorldTransformChangedEmitterID(UINT emitterID);

protected:
	std::vector<SEmitterForceProperty> m_forcePropertyCPU;
	std::vector<UINT> m_forcePropertyChangedEmitterIDs;
	std::unique_ptr<D3D11::CStructuredBuffer> m_forcePropertyGPU;

public:
	void AddForceChangedEmitterID(UINT emitterID);

protected:
	std::vector<UINT> m_interpInformationChangedEmitterIDs;

public:
	std::unique_ptr<D3D11::CStructuredBuffer> m_emitterInterpInformationGPU;

public:
	void AddInterpolaterInformChangedEmitterID(UINT emitterID);

protected:
	std::unique_ptr<D3D11::CAppendBuffer> m_aliveIndexSet;

protected:
	std::unique_ptr<D3D11::CDynamicBuffer> m_dispatchIndirectStagingBuffer;
	std::unique_ptr<D3D11::CStructuredBuffer> m_dispatchIndirectCalculatedBuffer;
	std::unique_ptr<D3D11::CIndirectBuffer<D3D11_DISPATCH_INDIRECT_ARGS>> m_dispatchIndirectBuffer;
	std::unique_ptr<D3D11::CIndirectBuffer<D3D11_DRAW_INSTANCED_INDIRECT_ARGS>> m_drawIndirectBuffer;

public:
	inline std::vector<std::unique_ptr<AEmitter>>& GetEmitters() noexcept { return m_emitters; };
	AEmitter* GetEmitter(UINT emitterID);
	bool FindEmitterFromID(UINT emitterID, OUT std::vector<std::unique_ptr<AEmitter>>::iterator& iter);

public:
	virtual UINT AddEmitter(
		DirectX::XMVECTOR position,
		DirectX::XMVECTOR angle,
		ID3D11Device* device, 
		ID3D11DeviceContext* deviceContext
	) = 0;
	void RemoveEmitter(UINT emitterID);

protected:
	std::unique_ptr<CGPUInterpPropertyManager<4, 2>> m_colorD1Dim4PorpertyManager;
	std::unique_ptr<CGPUInterpPropertyManager<4, 4>> m_colorD3Dim4PorpertyManager;

protected:
	void SelectColorGPUInterpolater(
		UINT emitterID,
		UINT colorInterpolaterID,
		bool isColorGPUInterpolaterOn,
		EInterpolationMethod colorInterpolationMethod,
		IInterpolater<4>* colorInterpolater
	);

protected:
	void UpdateColorGPUInterpolater(
		UINT emitterID,
		UINT colorInterpolaterID, 
		bool isColorGPUInterpolaterOn, 
		float maxLife, 
		EInterpolationMethod colorInterpolationMethod, 
		IInterpolater<4>* colorInterpolater
	);
	virtual void UpdateColorGPUInterpolaterImpl(
		UINT emitterID,
		UINT colorInterpolaterID,
		bool isColorGPUInterpolaterOn,
		float maxLife,
		EInterpolationMethod colorInterpolationMethod,
		IInterpolater<4>* colorInterpolater
	) = 0;

public:
	void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override final;
	void Update(ID3D11DeviceContext* deviceContext, float dt) override final;

protected:
	virtual void InitializeImpl(ID3D11Device* device, ID3D11DeviceContext* deviceContext);
	virtual void UpdateImpl(ID3D11DeviceContext* deviceContext, float dt);

public:
	virtual void InitializeAliveFlag(ID3D11DeviceContext* deviceContext) = 0;
	virtual void SourceParticles(ID3D11DeviceContext* deviceContext);
	virtual void CalculateForces(ID3D11DeviceContext* deviceContext);
	virtual void DrawEmitters(ID3D11DeviceContext* deviceContext);
	virtual void DrawParticles(ID3D11DeviceContext* deviceContext) = 0;
};