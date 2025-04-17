#pragma once
#include "BaseSelector.h"
#include "InterpolateHelper.h"

class InterpolationSelector : public BaseSelector<EInterpolationMethod>
{
public:
	static std::unordered_map<EInterpolationMethod, std::string> GInterpolationMethodStringMap;

public:
	static void GridViewControlPoint1s(
		const std::string& controlPointsName, 
		const std::vector<SControlPoint>& controlPoints
	);

public:
	static void ViewInterpolatedPoints(
		AInterpolater* interpolater,
		const std::string& graphTitle,
		const std::string& scatterLabels,
		const std::string& lineLabels,
		const std::vector<SControlPoint>& controlPoints
	);

private:
	static std::vector<float> MakeUniformStepsFromX(const std::vector<float>& xs, size_t numSteps);
};

