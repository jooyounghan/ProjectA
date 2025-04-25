#pragma once
#include <vector>
#include <queue>
#include <memory>

#include "DefineLinkedWithShader.h"
#include "BufferMacroUtilities.h"
#include "StructuredBuffer.h"

#define MaxStepCount MaxControlPointsCount - 1

template<uint32_t Dim, uint32_t CoefficientCount>
struct SInterpProperty
{
	struct
	{
		uint32_t controlPointsCount;
		uint32_t interpolaterFlag;
	} header;
	float xProfiles[MaxControlPointsCount];
	float coefficients[MaxStepCount][Dim][CoefficientCount];

	void UpdateInterpolaterProperty(
		uint32_t interpolaterFlagIn,
		const std::vector<float>& xProfilesIn,
		const std::vector<std::array<float, CoefficientCount* Dim>> coefficientsIn
	);
};

template<uint32_t Dim, uint32_t CoefficientCount>
void SInterpProperty<Dim, CoefficientCount>::UpdateInterpolaterProperty(
	uint32_t interpolaterFlagIn, 
	const std::vector<float>& xProfilesIn, 
	const std::vector<std::array<float, CoefficientCount* Dim>> coefficientsIn
)
{
	header.controlPointsCount = static_cast<uint32_t>(xProfilesIn.size());
	header.interpolaterFlag = interpolaterFlagIn;
	memcpy(xProfiles, xProfilesIn.data(), sizeof(float) * xProfilesIn.size());

	for (size_t stepIdx = 0; stepIdx < coefficientsIn.size(); ++stepIdx)
	{
		const std::array<float, CoefficientCount* Dim>& coefficientIn = coefficientsIn[stepIdx];
		for (size_t dimension = 0; dimension < Dim; ++dimension)
		{
			for (size_t coeffIdx = 0; coeffIdx < CoefficientCount; ++coeffIdx)
			{
				coefficients[stepIdx][dimension][coeffIdx] = coefficientIn[dimension * CoefficientCount + coeffIdx];
			}
		}
	}
}

template<uint32_t Dim, uint32_t CoefficientCount>
class CGPUInterpolater
{
private:
	struct InternalData
	{
		static std::vector<SInterpProperty<Dim, CoefficientCount>> GInterpPropertyCPU;
		static std::queue<uint32_t> GInterpPropertyIDQueue;
		static std::vector<uint32_t> GChangedInterpPropertyIDs;
	};

public:
	static std::unique_ptr<D3D11::CStructuredBuffer> GInterpPropertyGPU;

public:
	static UINT IssueAvailableInterpPropertyID();
	static void ReclaimInterpPropertyID(UINT interpPropertyID) noexcept;
	static SInterpProperty<Dim, CoefficientCount>* GetInterpProperty(UINT interpPropertyID);
	static void AddChangedEmitterInterpPropertyID(UINT interpPropertyID);

public:
	static void InitializeGPUInterpProperty(ID3D11Device* device, uint32_t maxEmitterCount);
	static void UpdateInterpProperty(ID3D11DeviceContext* deviceContext);
};

template<uint32_t Dim, uint32_t CoefficientCount>
std::vector<SInterpProperty<Dim, CoefficientCount>> CGPUInterpolater<Dim, CoefficientCount>::InternalData::GInterpPropertyCPU;

template<uint32_t Dim, uint32_t CoefficientCount>
std::queue<uint32_t> CGPUInterpolater<Dim, CoefficientCount>::InternalData::GInterpPropertyIDQueue;

template<uint32_t Dim, uint32_t CoefficientCount>
std::vector<uint32_t> CGPUInterpolater<Dim, CoefficientCount>::InternalData::GChangedInterpPropertyIDs;

template<uint32_t Dim, uint32_t CoefficientCount>
std::unique_ptr<D3D11::CStructuredBuffer> CGPUInterpolater<Dim, CoefficientCount>::GInterpPropertyGPU;

template<uint32_t Dim, uint32_t CoefficientCount>
inline UINT CGPUInterpolater<Dim, CoefficientCount>::IssueAvailableInterpPropertyID()
{
	if (InternalData::GInterpPropertyIDQueue.empty()) { throw std::exception("No Interpolater ID To Issue"); }

	UINT interpolaterID = InternalData::GInterpPropertyIDQueue.front();
	InternalData::GInterpPropertyIDQueue.pop();

	return interpolaterID;
}

template<uint32_t Dim, uint32_t CoefficientCount>
void CGPUInterpolater<Dim, CoefficientCount>::ReclaimInterpPropertyID(UINT interpPropertyID) noexcept
{
	InternalData::GInterpPropertyIDQueue.push(interpPropertyID);
}

template<uint32_t Dim, uint32_t CoefficientCount>
SInterpProperty<Dim, CoefficientCount>* CGPUInterpolater<Dim, CoefficientCount>::GetInterpProperty(UINT interpPropertyID)
{
	if (InternalData::GInterpPropertyCPU.size() > interpPropertyID)
	{
		return &InternalData::GInterpPropertyCPU[interpPropertyID];
	}
	return nullptr;
}

template<uint32_t Dim, uint32_t CoefficientCount>
inline void CGPUInterpolater<Dim, CoefficientCount>::AddChangedEmitterInterpPropertyID(UINT interpPropID)
{
	InternalData::GChangedInterpPropertyIDs.emplace_back(interpPropID);
}

template<uint32_t Dim, uint32_t CoefficientCount>
void CGPUInterpolater<Dim, CoefficientCount>::InitializeGPUInterpProperty(ID3D11Device* device, uint32_t maxEmitterCount)
{
	for (uint32_t idx = 0; idx < maxEmitterCount; ++idx)
	{
		InternalData::GInterpPropertyIDQueue.push(idx);
	}

	InternalData::GInterpPropertyCPU.resize(maxEmitterCount);
	GInterpPropertyGPU = std::make_unique<D3D11::CStructuredBuffer>(
		static_cast<UINT>(sizeof(SInterpProperty<Dim, CoefficientCount>)),
		maxEmitterCount,
		InternalData::GInterpPropertyCPU.data()
		);
	GInterpPropertyGPU->InitializeBuffer(device);
}

template<uint32_t Dim, uint32_t CoefficientCount>
void CGPUInterpolater<Dim, CoefficientCount>::UpdateInterpProperty(ID3D11DeviceContext* deviceContext)
{
	UINT changedInterpolaterIDsCount = static_cast<UINT>(InternalData::GChangedInterpPropertyIDs.size());
	if (changedInterpolaterIDsCount > 0)
	{
		GInterpPropertyGPU->StageNthElement(deviceContext, InternalData::GChangedInterpPropertyIDs.data(), changedInterpolaterIDsCount);
		GInterpPropertyGPU->UploadNthElement(deviceContext, InternalData::GChangedInterpPropertyIDs.data(), changedInterpolaterIDsCount);
		InternalData::GChangedInterpPropertyIDs.clear();
	}
}