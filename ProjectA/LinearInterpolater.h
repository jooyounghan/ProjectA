#pragma once
#include "Interpolater.h"

template<uint32_t Dim>
class LinearInterpolater : public AInterpolater<Dim, 2>
{
public:
	LinearInterpolater(
		const SControlPoint<Dim>& startPoint,
		const SControlPoint<Dim>& endPoint,
		const std::vector<SControlPoint<Dim>>& controlPoints
	);
	~LinearInterpolater() override = default;

protected:
	using Parent = AInterpolater<Dim, 2>;

public:
	virtual std::array<float, Dim> GetInterpolated(float x) noexcept override;
};

template<uint32_t Dim>
inline LinearInterpolater<Dim>::LinearInterpolater(
	const SControlPoint<Dim>& startPoint, 
	const SControlPoint<Dim>& endPoint, 
	const std::vector<SControlPoint<Dim>>& controlPoints
)
	: AInterpolater<Dim, 2>(startPoint, endPoint, controlPoints)
{
	// 최소 2개의 N개 Control Point 확보
	std::vector<SControlPoint<Dim>> linearContorlPoints = Parent::GetControlPoints(startPoint, endPoint, controlPoints);

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
		Parent::coefficients.emplace_back(coefficient);
    }
}

template<uint32_t Dim>
inline std::array<float, Dim> LinearInterpolater<Dim>::GetInterpolated(float x) noexcept
{
	std::array<float, Dim> result;
    size_t coefficientIndex = Parent::GetCoefficientIndex(x);
	const std::array<float, 2 * Dim>& coefficient = Parent::coefficients[coefficientIndex];
	for (uint32_t dimension = 0; dimension < Dim; ++dimension)
	{
		result[dimension] = coefficient[2 * dimension] * x + coefficient[2 * dimension + 1];
	}
	return result;
}
