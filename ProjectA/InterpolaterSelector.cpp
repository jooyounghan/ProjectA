#include "InterpolaterSelector.h"

using namespace std;

unordered_map<EInterpolationMethod, string> InterpolaterSelectorHelper::GInterpolationMethodStringMap
{
	{ EInterpolationMethod::Linear, "���� ����" },
	{ EInterpolationMethod::CubicSpline, "Cubic Spline ����" },
	{ EInterpolationMethod::CatmullRom, "CatmullRom ����" },
};
