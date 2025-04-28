#pragma once
#include "Interpolater.h"

template<uint32_t Dim>
class CLinearInterpolater : public AInterpolater<Dim, 2>
{
public:
	CLinearInterpolater(
		const SControlPoint<Dim>& startPoint,
		const SControlPoint<Dim>& endPoint,
		const std::vector<SControlPoint<Dim>>& controlPoints
	);
	~CLinearInterpolater() override = default;

protected:
	using Parent = AInterpolater<Dim, 2>;

public:
	virtual UINT GetInterpolaterFlag() override;
	virtual void UpdateCoefficient() override;

protected:
	virtual std::array<float, Dim> GetInterpolated(float x) noexcept override;
};

template<uint32_t Dim>
inline CLinearInterpolater<Dim>::CLinearInterpolater(
	const SControlPoint<Dim>& startPoint, 
	const SControlPoint<Dim>& endPoint, 
	const std::vector<SControlPoint<Dim>>& controlPoints
)
	: AInterpolater<Dim, 2>(startPoint, endPoint, controlPoints)
{
	UpdateCoefficient();
}

template<uint32_t Dim>
UINT CLinearInterpolater<Dim>::GetInterpolaterFlag()
{
	return 1;
}

template<uint32_t Dim>
inline void CLinearInterpolater<Dim>::UpdateCoefficient()
{
	Parent::UpdateCoefficient();

	// 최소 2개의 N개 Control Point 확보
	Parent::m_coefficients.clear();

	std::vector<SControlPoint<Dim>> linearContorlPoints = Parent::GetControlPoints();

	const size_t lineControlPointsStepCount = linearContorlPoints.size() - 1;
	for (size_t idx = 0; idx < lineControlPointsStepCount; ++idx)
	{
		const SControlPoint<Dim>& point1 = linearContorlPoints[idx];
		const SControlPoint<Dim>& point2 = linearContorlPoints[idx + 1];

		std::array<float, 2 * Dim> coefficient;
		for (uint32_t dimension = 0; dimension < Dim; ++dimension)
		{
			coefficient[2 * dimension] = (point1.y[dimension] - point2.y[dimension]) / (point1.x - point2.x);
			coefficient[2 * dimension + 1] = (point1.y[dimension] + point2.y[dimension] - coefficient[2 * dimension] * (point1.x + point2.x)) / 2.f;
		}
		Parent::m_coefficients.emplace_back(coefficient);
	}
}

template<uint32_t Dim>
inline std::array<float, Dim> CLinearInterpolater<Dim>::GetInterpolated(float x) noexcept
{
	std::array<float, Dim> result;
    size_t coefficientIndex = Parent::GetCoefficientIndex(x);
	const std::array<float, 2 * Dim>& coefficient = Parent::m_coefficients[coefficientIndex];
	for (uint32_t dimension = 0; dimension < Dim; ++dimension)
	{
		result[dimension] = coefficient[2 * dimension] * x + coefficient[2 * dimension + 1];
	}
	return result;
}
