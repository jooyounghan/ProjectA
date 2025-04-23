#pragma once
#include <vector>
#include <queue>
#include <memory>

#include "DefineLinkedWithShader.h"
#include "BufferMacroUtilities.h"
#include "StructuredBuffer.h"

#define MaxStepCount MaxControlPointsCount - 1

template<uint32_t Dim, uint32_t CoefficientCount>
struct SInterpolaterProperty
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
void SInterpolaterProperty<Dim, CoefficientCount>::UpdateInterpolaterProperty(
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
class GPUInterpolater
{
private:
	struct InternalData
	{
		static std::vector<SInterpolaterProperty<Dim, CoefficientCount>> GInterpolaterPropertyCPU;
		static std::queue<uint32_t> GInterpolaterIDQueue;
	};

public:
	static std::vector<uint32_t> GChangedInterpolaterIDs;

public:
	static std::unique_ptr<D3D11::CStructuredBuffer> GInterpolaterPropertyGPU;

public:
	static void InitializeGPUInterpolater(ID3D11Device* device, uint32_t maxEmitterCount);
	static UINT IssueAvailableInterpolaterID();
	static void ReclaimInterpolaterID(UINT interpolaterID) noexcept;
	static SInterpolaterProperty<Dim, CoefficientCount>* GetInterpolaterProperty(UINT interpolaterID);
	static void UpdateInterpolaterProperty(ID3D11DeviceContext* deviceContext);
};

template<uint32_t Dim, uint32_t CoefficientCount>
std::vector<SInterpolaterProperty<Dim, CoefficientCount>> GPUInterpolater<Dim, CoefficientCount>::InternalData::GInterpolaterPropertyCPU;

template<uint32_t Dim, uint32_t CoefficientCount>
std::queue<uint32_t> GPUInterpolater<Dim, CoefficientCount>::InternalData::GInterpolaterIDQueue;

template<uint32_t Dim, uint32_t CoefficientCount>
std::vector<uint32_t> GPUInterpolater<Dim, CoefficientCount>::GChangedInterpolaterIDs;

template<uint32_t Dim, uint32_t CoefficientCount>
std::unique_ptr<D3D11::CStructuredBuffer> GPUInterpolater<Dim, CoefficientCount>::GInterpolaterPropertyGPU;

template<uint32_t Dim, uint32_t CoefficientCount>
void GPUInterpolater<Dim, CoefficientCount>::InitializeGPUInterpolater(ID3D11Device* device, uint32_t maxEmitterCount)
{
	for (uint32_t idx = 0; idx < maxEmitterCount; ++idx)
	{
		InternalData::GInterpolaterIDQueue.push(idx);
	}

	InternalData::GInterpolaterPropertyCPU.resize(maxEmitterCount);
	GInterpolaterPropertyGPU = std::make_unique<D3D11::CStructuredBuffer>(
		static_cast<UINT>(sizeof(SInterpolaterProperty<Dim, CoefficientCount>)),
		maxEmitterCount,
		InternalData::GInterpolaterPropertyCPU.data()
	);
	GInterpolaterPropertyGPU->InitializeBuffer(device);
}

template<uint32_t Dim, uint32_t CoefficientCount>
inline UINT GPUInterpolater<Dim, CoefficientCount>::IssueAvailableInterpolaterID()
{
	if (InternalData::GInterpolaterIDQueue.empty()) { throw std::exception("No Interpolater ID To Issue"); }

	UINT interpolaterID = InternalData::GInterpolaterIDQueue.front();
	InternalData::GInterpolaterIDQueue.pop();

	return interpolaterID;
}

template<uint32_t Dim, uint32_t CoefficientCount>
void GPUInterpolater<Dim, CoefficientCount>::ReclaimInterpolaterID(UINT interpolaterID) noexcept
{
	InternalData::GInterpolaterIDQueue.push(interpolaterID);
}

template<uint32_t Dim, uint32_t CoefficientCount>
SInterpolaterProperty<Dim, CoefficientCount>* GPUInterpolater<Dim, CoefficientCount>::GetInterpolaterProperty(UINT interpolaterID)
{
	if (InternalData::GInterpolaterPropertyCPU.size() > interpolaterID)
	{
		return &InternalData::GInterpolaterPropertyCPU[interpolaterID];
	}
	return nullptr;
}

template<uint32_t Dim, uint32_t CoefficientCount>
void GPUInterpolater<Dim, CoefficientCount>::UpdateInterpolaterProperty(ID3D11DeviceContext* deviceContext)
{
	UINT changedInterpolaterIDsCount = static_cast<UINT>(GChangedInterpolaterIDs.size());
	if (changedInterpolaterIDsCount > 0)
	{
		GInterpolaterPropertyGPU->StageNthElement(deviceContext, GChangedInterpolaterIDs.data(), changedInterpolaterIDsCount);
		GInterpolaterPropertyGPU->UploadNthElement(deviceContext, GChangedInterpolaterIDs.data(), changedInterpolaterIDsCount);
		GChangedInterpolaterIDs.clear();
	}
}