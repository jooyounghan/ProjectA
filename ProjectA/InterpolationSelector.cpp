#include "InterpolationSelector.h"

using namespace std;

unordered_map<EInterpolationMethod, string> InterpolationSelector::GInterpolationMethodStringMap
{
	{ EInterpolationMethod::Linear, "선형 보간" },
	{ EInterpolationMethod::CubicSpline, "Cubic Spline 보간" }
};

vector<float> InterpolationSelector::MakeUniformStepsFromX(const vector<float>& xs, size_t numSteps)
{
	vector<float> result;

	float start = xs.front();
	float end = xs.back();
	float step = (end - start) / static_cast<float>(numSteps);

	result.reserve(numSteps + 1);
	for (size_t i = 0; i <= numSteps; ++i)
	{
		result.emplace_back(start + step * i);
	}

	return result;
}
