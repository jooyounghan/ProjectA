#pragma once
#include "DefineLinkedWithShader.h"

#include <vector>
#include <array>

#define MaxStepCount MaxControlPointsCount - 1
#define InterpPropertyNotSelect ~0u

enum class EInterpolationMethod
{
	Linear,
	CubicSpline,
	CatmullRom
};

template<uint32_t Dim>
struct SControlPoint
{
	float x = 0.f;
	std::array<float, Dim> y;
};


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
		const float* xProfilesAddress,
		size_t xProfilesCount,
		const float* coefficientsAddress,
		size_t coefficientsCount
	);
};

template<uint32_t Dim, uint32_t CoefficientCount>
void SInterpProperty<Dim, CoefficientCount>::UpdateInterpolaterProperty(
	uint32_t interpolaterFlagIn,
	const float* xProfilesAddress,
	size_t xProfilesCount,
	const float* coefficientsAddress,
	size_t coefficientsCount
)
{
	header.controlPointsCount = static_cast<uint32_t>(xProfilesCount);
	header.interpolaterFlag = interpolaterFlagIn;
	memcpy(xProfiles, xProfilesAddress, sizeof(float) * xProfilesCount);

	size_t offset = CoefficientCount * Dim;
	size_t stepCount = coefficientsCount / offset;
	for (size_t stepIdx = 0; stepIdx < stepCount; ++stepIdx)
	{
		float* coefficient = &coefficients[stepIdx][0][0];
		memcpy(coefficient, coefficientsAddress + offset * stepIdx, sizeof(float) * offset);
	}
}
