#pragma once
#include <vector>
#include <queue>
#include <memory>

#include "DefineLinkedWithShader.h"
#include "BufferMacroUtilities.h"
#include "StructuredBuffer.h"

template<uint32_t Dim, uint32_t CoefficientCount>
struct SInterpolaterProperty
{
	struct
	{
		uint32_t controlPointsCount;
		const uint32_t dimension = Dim;
		const uint32_t coefficientCount = CoefficientCount;
		uint32_t interpolaterFlag;
	} header;
	float xProfiles[MaxControlPointsCount];
	float coefficient[Dim][CoefficientCount];
};


template<uint32_t Dim, uint32_t CoefficientCount>
class GPUInterpolater
{
protected:
	static std::vector<SInterpolaterProperty<Dim, CoefficientCount>> GInterpolaterPropertyCPU;
	static std::queue<uint32_t> GInterpolaterIDQueue;

public:
	static std::unique_ptr<D3D11::CStructuredBuffer> GInterpolaterPropertyGPU;

public:
	static void InitializeGPUInterpolater(ID3D11Device* device, uint32_t maxEmitterCount);
	static UINT IssueAvailableInterpolaterID();
	static void ReclaimInterpolaterID(UINT interpolaterID) noexcept;

public:
	static void UpdateInterpolater(UINT interpolaterID);
};

template<uint32_t Dim, uint32_t CoefficientCount>
void GPUInterpolater<Dim, CoefficientCount>::InitializeGPUInterpolater(ID3D11Device* device, uint32_t maxEmitterCount)
{
	for (uint32_t idx = 0; idx < maxEmitterCount; ++idx)
	{
		GInterpolaterIDQueue.push(idx);
	}

	GInterpolaterPropertyCPU.resize(maxEmitterCount);
	GInterpolaterPropertyGPU = std::make_unique<D3D11::CStructuredBuffer>(
		static_cast<UINT>(sizeof(SInterpolaterProperty<Dim, CoefficientCount>)),
		maxEmitterCount,
		GInterpolaterPropertyCPU.data()
	);
	GInterpolaterPropertyGPU->InitializeBuffer(device);
}

template<uint32_t Dim, uint32_t CoefficientCount>
inline UINT GPUInterpolater<Dim, CoefficientCount>::IssueAvailableInterpolaterID()
{
	if (GInterpolaterIDQueue.empty()) { throw std::exception("No Interpolater ID To Issue"); }

	UINT interpolaterID = GInterpolaterIDQueue.front();
	GInterpolaterIDQueue.pop();

	return interpolaterID;
}

template<uint32_t Dim, uint32_t CoefficientCount>
void GPUInterpolater<Dim, CoefficientCount>::ReclaimInterpolaterID(UINT interpolaterID) noexcept
{
	GInterpolaterIDQueue.push(interpolaterID);
}

template<uint32_t Dim, uint32_t CoefficientCount>
void GPUInterpolater<Dim, CoefficientCount>::UpdateInterpolater(UINT interpolaterID)
{
}