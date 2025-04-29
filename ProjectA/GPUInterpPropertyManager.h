#pragma once
#include <queue>
#include <memory>

#include "IUpdatable.h"
#include "InterpolaterStructure.h"
#include "StructuredBuffer.h"

template<uint32_t Dim, uint32_t CoefficientCount>
class CGPUInterpPropertyManager : public IUpdatable
{
public:
	CGPUInterpPropertyManager(uint32_t maxEmitterCount);
	~CGPUInterpPropertyManager() override = default;

protected:
	uint32_t m_maxEmitterCount;

protected:
	std::vector<SInterpProperty<Dim, CoefficientCount>> m_interpPropertyCPU;
	std::queue<uint32_t> m_interpPropertyIDQueue;
	std::vector<uint32_t> m_changedInterpPropertyIDs;

public:
	std::unique_ptr<D3D11::CStructuredBuffer> m_interpPropertyGPU;

public:
	inline ID3D11ShaderResourceView* GetGPUInterpPropertySRV() noexcept { return m_interpPropertyGPU->GetSRV(); }

public:
	UINT IssueAvailableInterpPropertyID();
	void ReclaimInterpPropertyID(UINT interpPropertyID) noexcept;
	void AddChangedEmitterInterpPropertyID(UINT interpPropertyID);
	SInterpProperty<Dim, CoefficientCount>* GetInterpProperty(UINT interpPropertyID);

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;
};

template<uint32_t Dim, uint32_t CoefficientCount>
inline CGPUInterpPropertyManager<Dim, CoefficientCount>::CGPUInterpPropertyManager(uint32_t maxEmitterCount)
	: m_maxEmitterCount(maxEmitterCount)
{
	for (uint32_t idx = 0; idx < maxEmitterCount; ++idx)
	{
		m_interpPropertyIDQueue.push(idx);
	}

	m_interpPropertyCPU.resize(maxEmitterCount);
}

template<uint32_t Dim, uint32_t CoefficientCount>
inline UINT CGPUInterpPropertyManager<Dim, CoefficientCount>::IssueAvailableInterpPropertyID()
{
	if (m_interpPropertyIDQueue.empty()) { throw std::exception("No Interpolater ID To Issue"); }

	UINT interpolaterID = m_interpPropertyIDQueue.front();
	m_interpPropertyIDQueue.pop();

	return interpolaterID;
}

template<uint32_t Dim, uint32_t CoefficientCount>
void CGPUInterpPropertyManager<Dim, CoefficientCount>::ReclaimInterpPropertyID(UINT interpPropertyID) noexcept
{
	if (interpPropertyID != InterpPropertyNotSelect)
	{
		m_interpPropertyIDQueue.push(interpPropertyID);
	}
}

template<uint32_t Dim, uint32_t CoefficientCount>
SInterpProperty<Dim, CoefficientCount>* CGPUInterpPropertyManager<Dim, CoefficientCount>::GetInterpProperty(UINT interpPropertyID)
{
	if (m_interpPropertyCPU.size() > interpPropertyID)
	{
		return &m_interpPropertyCPU[interpPropertyID];
	}
	return nullptr;
}

template<uint32_t Dim, uint32_t CoefficientCount>
inline void CGPUInterpPropertyManager<Dim, CoefficientCount>::AddChangedEmitterInterpPropertyID(UINT interpPropID)
{
	m_changedInterpPropertyIDs.emplace_back(interpPropID);
}

template<uint32_t Dim, uint32_t CoefficientCount>
void CGPUInterpPropertyManager<Dim, CoefficientCount>::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{
	m_interpPropertyGPU = std::make_unique<D3D11::CStructuredBuffer>(
		static_cast<UINT>(sizeof(SInterpProperty<Dim, CoefficientCount>)),
		m_maxEmitterCount,
		m_interpPropertyCPU.data()
	);
	m_interpPropertyGPU->InitializeBuffer(device);
}

template<uint32_t Dim, uint32_t CoefficientCount>
void CGPUInterpPropertyManager<Dim, CoefficientCount>::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	UINT changedInterpolaterIDsCount = static_cast<UINT>(m_changedInterpPropertyIDs.size());
	if (changedInterpolaterIDsCount > 0)
	{
		m_interpPropertyGPU->StageNthElement(deviceContext, m_changedInterpPropertyIDs.data(), changedInterpolaterIDsCount);
		m_interpPropertyGPU->UploadNthElement(deviceContext, m_changedInterpPropertyIDs.data(), changedInterpolaterIDsCount);
		m_changedInterpPropertyIDs.clear();
	}
}