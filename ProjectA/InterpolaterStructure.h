#pragma once
#include <array>

enum class EInterpolationMethod
{
	Linear,
	CubicSpline,
	CatmullRom
};

template<uint32_t Dim>
struct SControlPoint
{
	float x = 0.f;
	std::array<float, Dim> y;
};