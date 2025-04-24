#pragma once
#include "Interpolater.h"


template<uint32_t Dim>
class CCubicSplineInterpolater : public AInterpolater<Dim, 4>
{
public:
	CCubicSplineInterpolater(
		bool useGPUInterpolater,
		const SControlPoint<Dim>& startPoint,
		const SControlPoint<Dim>& endPoint,
		const std::vector<SControlPoint<Dim>>& controlPoints
	);
	~CCubicSplineInterpolater() override = default;

protected:
	using Parent = AInterpolater<Dim, 4>;

public:
	virtual UINT GetInterpolaterFlag() override;
	virtual void UpdateCoefficient() override;
	virtual std::array<float, Dim> GetInterpolated(float x) noexcept override;

protected:
	std::vector<std::array<float, Dim>> GetSecondDerivative(
		const std::vector<std::array<float, Dim>>& L,
		const std::vector<std::array<float, Dim>>& D,
		const std::vector<std::array<float, Dim>>& U,
		const std::vector<std::array<float, Dim>>& Derivative
	);
};

template<uint32_t Dim>
inline CCubicSplineInterpolater<Dim>::CCubicSplineInterpolater(
	bool useGPUInterpolater,
	const SControlPoint<Dim>& startPoint,
	const SControlPoint<Dim>& endPoint,
	const std::vector<SControlPoint<Dim>>& controlPoints
)
	: AInterpolater<Dim, 4>(useGPUInterpolater, startPoint, endPoint, controlPoints)
{
	UpdateCoefficient();
}

template<uint32_t Dim>
UINT CCubicSplineInterpolater<Dim>::GetInterpolaterFlag()
{
	return 2;
}

template<uint32_t Dim>
inline void CCubicSplineInterpolater<Dim>::UpdateCoefficient()
{
	Parent::UpdateCoefficient();
	Parent::m_coefficients.clear();

	std::vector<SControlPoint<Dim>> cubicSplineContorlPoints = Parent::GetControlPoints();
	const size_t cubicSplineContorlPointsStepCount = cubicSplineContorlPoints.size() - 1;

	for (size_t idx = 0; idx < cubicSplineContorlPointsStepCount; ++idx)
	{
		std::array<float, 4 * Dim> coefficient;
		for (uint32_t dimension = 0; dimension < Dim; ++dimension)
		{
			coefficient[4 * dimension + 3] = cubicSplineContorlPoints[idx].y[dimension];
		}
		Parent::m_coefficients.emplace_back(coefficient);
	}

	std::vector<float> stepSizes(cubicSplineContorlPointsStepCount);
	for (size_t idx = 0; idx < cubicSplineContorlPointsStepCount; ++idx)
	{
		stepSizes[idx] = cubicSplineContorlPoints[idx + 1].x - cubicSplineContorlPoints[idx].x;
	}

	std::vector<std::array<float, Dim>> secondDerivative(cubicSplineContorlPointsStepCount + 1);
	secondDerivative[0] = MakeZeroArray<Dim>();
	secondDerivative[cubicSplineContorlPointsStepCount] = MakeZeroArray<Dim>();

	std::vector<std::array<float, Dim>> L(cubicSplineContorlPointsStepCount - 1);
	std::vector<std::array<float, Dim>> D(cubicSplineContorlPointsStepCount - 1);
	std::vector<std::array<float, Dim>> U(cubicSplineContorlPointsStepCount - 1);
	std::vector<std::array<float, Dim>> derivative(cubicSplineContorlPointsStepCount - 1);


	for (size_t idx = 1; idx < cubicSplineContorlPointsStepCount; ++idx)
	{
		const std::array<float, Dim>& y2 = cubicSplineContorlPoints[idx + 1].y;
		const std::array<float, Dim>& y1 = cubicSplineContorlPoints[idx].y;
		const std::array<float, Dim>& y0 = cubicSplineContorlPoints[idx - 1].y;

		const float step1 = stepSizes[idx];
		const float step0 = stepSizes[idx - 1];

		for (uint32_t dimension = 0; dimension < Dim; ++dimension)
		{
			L[idx - 1][dimension] = step0;
			D[idx - 1][dimension] = 2.f * (step0 + step1);
			U[idx - 1][dimension] = step1;
			derivative[idx - 1][dimension] = 6.f * ((y2[dimension] - y1[dimension]) / step1 - (y1[dimension] - y0[dimension]) / step0);
		}
	}

	if (cubicSplineContorlPointsStepCount > 2)
	{
		std::vector<std::array<float, Dim>> innerSecondDerivative = GetSecondDerivative(L, D, U, derivative);
		for (size_t idx = 1; idx < cubicSplineContorlPointsStepCount; ++idx)
		{
			secondDerivative[idx] = innerSecondDerivative[idx - 1];
		}
	}
	else if (cubicSplineContorlPointsStepCount == 2)
	{
		for (size_t dimension = 0; dimension < Dim; ++dimension)
		{
			secondDerivative[1][dimension] = derivative[0][dimension] / D[0][dimension];
		}
	}
	else;

	for (size_t idx = 0; idx < cubicSplineContorlPointsStepCount; ++idx)
	{
		for (uint32_t dimension = 0; dimension < Dim; ++dimension)
		{
			std::array<float, 4 * Dim>& coefficient = Parent::m_coefficients[idx];
			coefficient[4 * dimension + 0] = (secondDerivative[idx + 1][dimension] - secondDerivative[idx][dimension]) / (6.f * stepSizes[idx]);
			coefficient[4 * dimension + 1] = secondDerivative[idx][dimension] / 2.f;
			coefficient[4 * dimension + 2] = (cubicSplineContorlPoints[idx + 1].y[dimension] - cubicSplineContorlPoints[idx].y[dimension]) / stepSizes[idx] - stepSizes[idx] * (secondDerivative[idx + 1][dimension] + 2.f * secondDerivative[idx][dimension]) / 6.f;
		}
	}

	if (Parent::m_interpolaterPropertyCached)
	{
		Parent::UpdateInterpolaterProperty();
	}
}

template<uint32_t Dim>
inline std::array<float, Dim> CCubicSplineInterpolater<Dim>::GetInterpolated(float x) noexcept
{
	size_t coefficientIndex = Parent::GetCoefficientIndex(x);

	float x1 = Parent::m_xProfiles[coefficientIndex];
	float t = x - x1;

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

template<uint32_t Dim>
inline std::vector<std::array<float, Dim>> CCubicSplineInterpolater<Dim>::GetSecondDerivative(
	const std::vector<std::array<float, Dim>>& L, 
	const std::vector<std::array<float, Dim>>& D, 
	const std::vector<std::array<float, Dim>>& U, 
	const std::vector<std::array<float, Dim>>& derivative
)
{
	int n = static_cast<int>(D.size());
	std::vector<std::array<float, Dim>> decomposedU(n);
	std::vector<std::array<float, Dim>> decomposedDerivative(n);

	for (uint32_t dimension = 0; dimension < Dim; ++dimension)
	{
		decomposedU[0][dimension] = U[0][dimension] / D[0][dimension];
		decomposedDerivative[0][dimension] = derivative[0][dimension] / D[0][dimension];
	}

	for (int idx = 1; idx < n - 1; ++idx)
	{
		for (uint32_t dimension = 0; dimension < Dim; ++dimension)
		{
			float m = D[idx][dimension] - L[idx][dimension] * decomposedU[idx - 1][dimension];
			decomposedU[idx][dimension] = U[idx][dimension] / m;
			decomposedDerivative[idx][dimension] = (derivative[idx][dimension] - L[idx][dimension] * decomposedDerivative[idx - 1][dimension]) / m;
		}
	}

	std::vector<std::array<float, Dim>> secondDerivative(n);
	for (uint32_t dimension = 0; dimension < Dim; ++dimension)
	{
		secondDerivative[n - 1][dimension] = (derivative[n - 1][dimension] - L[n - 1][dimension] * decomposedDerivative[n - 2][dimension]) /
			(D[n - 1][dimension] - L[n - 1][dimension] * decomposedU[n - 2][dimension]);
	}

	for (int idx = static_cast<int>(n - 2); idx >= 0; --idx)
	{
		for (uint32_t dimension = 0; dimension < Dim; ++dimension)
		{
			secondDerivative[idx][dimension] = decomposedDerivative[idx][dimension] - decomposedU[idx][dimension] * secondDerivative[idx + 1][dimension];
		}
	}
	return secondDerivative;
}
