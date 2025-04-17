#pragma once
#include "BaseSelector.h"
#include "LinearInterpolater.h"
#include "CubicSplineInterpolater.h"

#include <memory>

enum class EInterpolationMethod
{
	Linear,
	CubicSpline
};

class InterpolationSelector : public BaseSelector<EInterpolationMethod>
{
public:
	static std::unordered_map<EInterpolationMethod, std::string> GInterpolationMethodStringMap;
	static std::unique_ptr<IInterpolater> CreateInterpolater(
		EInterpolationMethod interpolationMethod,
		const SControlPoint& startPoint,
		const SControlPoint& endPoint,
		const std::vector<SControlPoint>& controlPoints
	);

public:
	static void GridViewControlPoints(
		const std::string& yValueName,
		const std::string& controlPointsName,
		const SControlPoint& startPoint,
		const SControlPoint& endPoint,
		const std::vector<SControlPoint>& controlPoints
	);

public:
	static void ViewInterpolatedPoints(
		IInterpolater* interpolater,
		const std::string& graphTitle,
		const std::string& scatterLabels,
		const std::string& lineLabels,
		const SControlPoint& startPoint,
		const SControlPoint& endPoint,
		const std::vector<SControlPoint>& controlPoints
	);

private:
	static std::vector<float> MakeUniformStepsFromX(const std::vector<float>& xs, size_t numSteps);
};

