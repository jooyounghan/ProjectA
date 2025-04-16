#include "InterpolateHelper.h"

using namespace std;
using namespace DirectX;

unordered_map<EInterpolationMethod, FPoint1Interpolater> InterpolateHelper::GPoint1InterpolaterMapper
{
	{ EInterpolationMethod::Linear, bind(InterpolateHelper::Point1LinearInterpolate, placeholders::_1, placeholders::_2) },
	{ EInterpolationMethod::CubicSpline, bind(InterpolateHelper::Point1CubieSplineInterpolate, placeholders::_1, placeholders::_2) }
};

unordered_map<EInterpolationMethod, FPoint2Interpolater> InterpolateHelper::GPoint2InterpolaterMapper
{
	{ EInterpolationMethod::Linear, bind(InterpolateHelper::Point2LinearInterpolate, placeholders::_1, placeholders::_2) },
	{ EInterpolationMethod::CubicSpline, bind(InterpolateHelper::Point2CubieSplineInterpolate, placeholders::_1, placeholders::_2) }
};

unordered_map<EInterpolationMethod, FPoint3Interpolater> InterpolateHelper::GPoint3InterpolaterMapper
{
	{ EInterpolationMethod::Linear, bind(InterpolateHelper::Point3LinearInterpolate, placeholders::_1, placeholders::_2) },
	{ EInterpolationMethod::CubicSpline, bind(InterpolateHelper::Point3CubieSplineInterpolate, placeholders::_1, placeholders::_2) }
};

float InterpolateHelper::Point1LinearInterpolate(const vector<SControlPoint1>& controlPoint1s, float x)
{
	if (controlPoint1s.size() > 0)
	{


		return 0.f;
	}
	else
	{
		throw exception("Linear Interpolation Need More Than One Control Point");
	}
}

float InterpolateHelper::Point1CubieSplineInterpolate(const vector<SControlPoint1>& controlPoint1s, float x)
{
	return 0.f;
}

XMFLOAT2 InterpolateHelper::Point2LinearInterpolate(const vector<SControlPoint2>& controlPoint1s, float x)
{
	return XMFLOAT2(0.f, 0.f);
}

XMFLOAT2 InterpolateHelper::Point2CubieSplineInterpolate(const vector<SControlPoint2>& controlPoint1s, float x)
{
	return XMFLOAT2(0.f, 0.f);
}

XMFLOAT3 InterpolateHelper::Point3LinearInterpolate(const vector<SControlPoint3>& controlPoint1s, float x)
{
	return XMFLOAT3(0.f, 0.f, 0.f);
}

XMFLOAT3 InterpolateHelper::Point3CubieSplineInterpolate(const vector<SControlPoint3>& controlPoint1s, float x)
{
	return XMFLOAT3(0.f, 0.f, 0.f);
}

FPoint1Interpolater InterpolateHelper::GetPoint1Interpolater(EInterpolationMethod interpolationMethod)
{
	if (GPoint1InterpolaterMapper.find(interpolationMethod) != GPoint1InterpolaterMapper.end())
	{
		return GPoint1InterpolaterMapper[interpolationMethod];
	}
	else
	{
		throw exception("No Interpolater matches With EInterpolateMethod");
	}
}

FPoint2Interpolater InterpolateHelper::GetPoint2Interpolater(EInterpolationMethod interpolationMethod)
{
	if (GPoint2InterpolaterMapper.find(interpolationMethod) != GPoint2InterpolaterMapper.end())
	{
		return GPoint2InterpolaterMapper[interpolationMethod];
	}
	else
	{
		throw exception("No Interpolater matches With EInterpolateMethod");
	}
}

FPoint3Interpolater InterpolateHelper::GetPoint3Interpolater(EInterpolationMethod interpolationMethod)
{
	if (GPoint3InterpolaterMapper.find(interpolationMethod) != GPoint3InterpolaterMapper.end())
	{
		return GPoint3InterpolaterMapper[interpolationMethod];
	}
	else
	{
		throw exception("No Interpolater matches With EInterpolateMethod");
	}
}
