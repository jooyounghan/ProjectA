#pragma once
#include "Interpolater.h"

template<uint32_t Dim>
class CubicSplineInterpolater : public AInterpolater<Dim, 4>
{
public:
	CubicSplineInterpolater(
		const SControlPoint<Dim>& startPoint,
		const SControlPoint<Dim>& endPoint,
		const std::vector<SControlPoint<Dim>>& controlPoints
	);
	~CubicSplineInterpolater() override = default;

protected:
	using Parent = AInterpolater<Dim, 4>;

protected:
	virtual std::vector<SControlPoint<Dim>> GetControlPoints() override;

public:
	virtual void UpdateCoefficient() override;
	virtual std::array<float, Dim> GetInterpolated(float x) noexcept override;
};

template<uint32_t Dim>
inline CubicSplineInterpolater<Dim>::CubicSplineInterpolater(
	const SControlPoint<Dim>& startPoint, 
	const SControlPoint<Dim>& endPoint, 
	const std::vector<SControlPoint<Dim>>& controlPoints
)
	: AInterpolater<Dim, 4>(startPoint, endPoint, controlPoints)
{
	UpdateCoefficient();
}

template<uint32_t Dim>
inline std::vector<SControlPoint<Dim>> CubicSplineInterpolater<Dim>::GetControlPoints()
{
	std::vector<SControlPoint<Dim>> cubicSplineContorlPoints = Parent::GetControlPoints();
	cubicSplineContorlPoints.insert(cubicSplineContorlPoints.begin(), SControlPoint<Dim>{ Parent::m_startPoint.x - 1.f, Parent::m_startPoint.y });
	cubicSplineContorlPoints.emplace_back(SControlPoint<Dim>{ Parent::m_endPoint.x + 1.f, Parent::m_endPoint.y });
	return cubicSplineContorlPoints;
}

template<uint32_t Dim>
inline void CubicSplineInterpolater<Dim>::UpdateCoefficient()
{
	Parent::UpdateCoefficient();

	// 최소 4개의 Control Point 확보
	Parent::m_coefficients.clear();

	std::vector<SControlPoint<Dim>> cubicSplineContorlPoints = GetControlPoints();

	const size_t cubicSplineContorlPointsStepCount = cubicSplineContorlPoints.size() - 3;
	for (size_t idx = 0; idx < cubicSplineContorlPointsStepCount; ++idx)
	{
		const std::array<float, Dim>& y0 = cubicSplineContorlPoints[idx].y;
		const std::array<float, Dim>& y1 = cubicSplineContorlPoints[idx + 1].y;
		const std::array<float, Dim>& y2 = cubicSplineContorlPoints[idx + 2].y;
		const std::array<float, Dim>& y3 = cubicSplineContorlPoints[idx + 3].y;

		std::array<float, 4 * Dim> coefficient;
		for (uint32_t dimension = 0; dimension < Dim; ++dimension)
		{
			coefficient[4 * dimension] = 0.5f * (-y0[dimension] + 3.0f * y1[dimension] - 3.0f * y2[dimension] + y3[dimension]);
			coefficient[4 * dimension + 1] = 0.5f * (2.0f * y0[dimension] - 5.0f * y1[dimension] + 4.0f * y2[dimension] - y3[dimension]);
			coefficient[4 * dimension + 2] = 0.5f * (-y0[dimension] + y2[dimension]);
			coefficient[4 * dimension + 3] = 0.5f * (2.0f * y1[dimension]);
		}
		Parent::m_coefficients.emplace_back(coefficient);
	}
}

template<uint32_t Dim>
inline std::array<float, Dim> CubicSplineInterpolater<Dim>::GetInterpolated(float x) noexcept
{
	size_t coefficientIndex = Parent::GetCoefficientIndex(x);
	
	float x1 = Parent::m_xProfiles[coefficientIndex];
	float x2 = Parent::m_xProfiles[coefficientIndex + 1];
	float t = (x - x1) / (x2 - x1);
	
	const std::array<float, 4 * Dim>& coefficient = Parent::m_coefficients[coefficientIndex];
	
	std::array<float, Dim> result;
	for (uint32_t dimension = 0; dimension < Dim; ++dimension)
	{
		float a = coefficient[4 * dimension + 0];
		float b = coefficient[4 * dimension + 1];
		float c = coefficient[4 * dimension + 2];
		float d = coefficient[4 * dimension + 3];
		result[dimension] = ((a * t + b) * t + c) * t + d;
	}
	return result;
}
