#pragma once
#include <vector>
#include <DirectXMath.h>

struct SControlPoint
{
	float x = 0.f;
	float y = 0.f;
};

class IInterpolater
{
public:
	virtual float GetInterpolated(float x) noexcept = 0;
};

template<typename T>
class AInterpolater : public IInterpolater
{
public:
	AInterpolater(
		const SControlPoint& startPoint, 
		const SControlPoint& endPoint, 
		const std::vector<SControlPoint>& controlPoints
	);
	virtual ~AInterpolater() = default;

protected:
	std::vector<float> xProfiles;
	std::vector<T> coefficients;

protected:
	virtual std::vector<SControlPoint> GetControlPoints(
		const SControlPoint& startPoint,
		const SControlPoint& endPoint,
		const std::vector<SControlPoint>& controlPoints
	);

protected:
	size_t GetCoefficientIndex(float x) noexcept;
};

template<typename T>
inline AInterpolater<T>::AInterpolater(
	const SControlPoint& startPoint, 
	const SControlPoint& endPoint, 
	const std::vector<SControlPoint>& controlPoints
)
{
	xProfiles.emplace_back(startPoint.x);
	for (const SControlPoint& controlPoint : controlPoints)
	{
		xProfiles.emplace_back(controlPoint.x);
	}
	xProfiles.emplace_back(endPoint.x);
}

template<typename T>
inline std::vector<SControlPoint> AInterpolater<T>::GetControlPoints(
	const SControlPoint& startPoint, 
	const SControlPoint& endPoint, 
	const std::vector<SControlPoint>& controlPoints
)
{
	std::vector<SControlPoint> result;
	result.reserve(2 + controlPoints.size());
	result.emplace_back(startPoint);
	result.insert(result.end(), controlPoints.begin(), controlPoints.end());
	result.emplace_back(endPoint);
	return result;
}

template<typename T>
inline size_t AInterpolater<T>::GetCoefficientIndex(float x) noexcept
{
	const size_t xProfileStepCounts = xProfiles.size() - 1;

	for (size_t idx = 0; idx < xProfileStepCounts; ++idx)
	{
		if (xProfiles[idx] <= x && x < xProfiles[idx + 1])
		{
			return idx;
		}
	}
	return xProfileStepCounts - 1;
}
