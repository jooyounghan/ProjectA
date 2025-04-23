#include "InterpolaterSelector.h"

using namespace std;

unordered_map<EInterpolationMethod, string> InterpolaterSelectorHelper::GInterpolationMethodStringMap
{
	{ EInterpolationMethod::Linear, "선형 보간" },
	{ EInterpolationMethod::CubicSpline, "Cubic Spline 보간" },
	{ EInterpolationMethod::CatmullRom, "CatmullRom 보간" },
};
