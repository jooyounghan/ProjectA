#include "InterpolationSelector.h"

using namespace std;

unordered_map<EInterpolationMethod, string> InterpolationSelectorHelper::GInterpolationMethodStringMap
{
	{ EInterpolationMethod::Linear, "���� ����" },
	{ EInterpolationMethod::CubicSpline, "Cubic Spline ����" }
};
