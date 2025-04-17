#include "InterpolateHelper.h"
#include <exception>

using namespace std;
using namespace DirectX;

bool AInterpolater::GetCoefficients(const std::vector<SControlPoint>& controlPoints) noexcept
{
	xs.clear();
	for (auto& controlPoint : controlPoints)
	{
		xs.emplace_back(controlPoint.x);
	}
	return true;
}

bool LinearInterpolater::IsInterpolatable(size_t controlPointsCount) noexcept
{
	return controlPointsCount > 1;
}

bool LinearInterpolater::GetCoefficients(const std::vector<SControlPoint>& controlPoints) noexcept
{
	AInterpolater::GetCoefficients(controlPoints);
	coefficients.clear();

	const size_t controlPointsCount = controlPoints.size();
	if (controlPointsCount < 2) return false;

	const size_t controlPointsStepCount = controlPointsCount - 2;
	for (size_t idx = 0; idx <= controlPointsStepCount; ++idx)
	{
		const SControlPoint& point1 = controlPoints[idx];
		const SControlPoint& point2 = controlPoints[idx + 1];

		XMFLOAT2 coefficient;
		coefficient.x = (point1.y - point2.y) / (point1.x - point2.x);
		coefficient.y = (point1.y + point2.y - coefficient.x * (point1.x + point2.x)) / 2.f;
		coefficients.emplace_back(coefficient);
	}
	return true;
}

float LinearInterpolater::GetInterpolated(float x) noexcept
{
	size_t intervalIndex = GetIntervalIndex(x);
	XMFLOAT2 coefficient = coefficients[intervalIndex];
	return coefficient.x * x + coefficient.y;
}

size_t LinearInterpolater::GetIntervalIndex(float x) noexcept
{
	const size_t controlPointsCount = xs.size();
	const size_t lastIndex = controlPointsCount - 2;

	for (size_t idx = 0; idx <= lastIndex; ++idx)
	{
		if (xs[idx] <= x && x < xs[idx + 1])
		{
			return idx;
		}
	}

	return lastIndex;
}

bool CubicSplineInterpolater::IsInterpolatable(size_t controlPointsCount) noexcept
{
	return controlPointsCount > 2;

}

bool CubicSplineInterpolater::GetCoefficients(const std::vector<SControlPoint>& controlPoints) noexcept
{
	AInterpolater::GetCoefficients(controlPoints);
	coefficients.clear();

	const size_t controlPointsCount = controlPoints.size();
	if (controlPointsCount < 3) return false;
	const size_t controlPointsStepCount = controlPointsCount - 2;
	for (size_t idx = 0; idx <= controlPointsStepCount; ++idx) 
	{
		const float y0 = (idx == 0) ? controlPoints[idx].y : controlPoints[idx - 1].y;
		const float y1 = controlPoints[idx].y;
		const float y2 = controlPoints[idx + 1].y;
		const float y3 = (idx + 2 < controlPointsCount) ? controlPoints[idx + 2].y : controlPoints[idx + 1].y;

		XMVECTOR coefficient = XMVectorSet(
			0.5f * (-y0 + 3.0f * y1 - 3.0f * y2 + y3),
			0.5f * (2.0f * y0 - 5.0f * y1 + 4.0f * y2 - y3),
			0.5f * (-y0 + y2),
			0.5f * (2.0f * y1)
		);

		coefficients.emplace_back(coefficient);
	}
	return true;
}

float CubicSplineInterpolater::GetInterpolated(float x) noexcept
{
	const size_t intervalIndex = GetIntervalIndex(x);

	float x1 = xs[intervalIndex];
	float x2 = xs[intervalIndex + 1];
	float t = (x - x1) / (x2 - x1);

	XMVECTOR coefficient = coefficients[intervalIndex];
	float a = XMVectorGetX(coefficient);
	float b = XMVectorGetY(coefficient);
	float c = XMVectorGetZ(coefficient);
	float d = XMVectorGetW(coefficient);
	return ((a * t + b) * t + c) * t + d;
}

size_t CubicSplineInterpolater::GetIntervalIndex(float x) noexcept
{
	const size_t controlPointsCount = xs.size();
	const size_t lastIndex = controlPointsCount - 2;

	for (size_t idx = 0; idx <= lastIndex; ++idx) 
	{
		if (xs[idx] <= x && x < xs[idx + 1])
		{
			return idx;
		}
	}

	return lastIndex;
}

unique_ptr<AInterpolater> InterpolaterHelper::GetInterpolater(EInterpolationMethod interpolationMethod)
{
	unique_ptr<AInterpolater> result;
	switch (interpolationMethod)
	{
	case EInterpolationMethod::None:
		result = nullptr;
		break;
	case EInterpolationMethod::Linear:
		result = make_unique<LinearInterpolater>();
		break;
	case EInterpolationMethod::CubicSpline:
		result = make_unique<CubicSplineInterpolater>();
		break;
	default:
		break;
	}

	return result;
}
