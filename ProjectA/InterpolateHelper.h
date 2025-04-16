#pragma once
#include <vector>
#include <functional>
#include <unordered_map>
#include <DirectXMath.h>

struct SControlPoint1
{
	float pointIn;
	float pointOut;
};

struct SControlPoint2
{
	float pointIn;
	DirectX::XMFLOAT2 point2Out;
};
struct SControlPoint3
{
	float pointIn;
	DirectX::XMFLOAT3 point3Out;
};


enum class EInterpolationMethod
{
	Linear,
	CubicSpline
};

typedef std::function<float(const std::vector<SControlPoint1>& controlPoints, float x)> FPoint1Interpolater;
typedef std::function<DirectX::XMFLOAT2(const std::vector<SControlPoint2>& controlPoints, float x)> FPoint2Interpolater;
typedef std::function<DirectX::XMFLOAT3(const std::vector<SControlPoint3>& controlPoints, float x)> FPoint3Interpolater;

class InterpolateHelper
{
public:
	static std::unordered_map<EInterpolationMethod, FPoint1Interpolater> GPoint1InterpolaterMapper;
	static std::unordered_map<EInterpolationMethod, FPoint2Interpolater> GPoint2InterpolaterMapper;
	static std::unordered_map<EInterpolationMethod, FPoint3Interpolater> GPoint3InterpolaterMapper;

public:
	static float Point1LinearInterpolate(const std::vector<SControlPoint1>& controlPoint1s, float x);
	static float Point1CubieSplineInterpolate(const std::vector<SControlPoint1>& controlPoint1s, float x);
	static DirectX::XMFLOAT2 Point2LinearInterpolate(const std::vector<SControlPoint2>& controlPoint1s, float x);
	static DirectX::XMFLOAT2 Point2CubieSplineInterpolate(const std::vector<SControlPoint2>& controlPoint1s, float x);
	static DirectX::XMFLOAT3 Point3LinearInterpolate(const std::vector<SControlPoint3>& controlPoint1s, float x);
	static DirectX::XMFLOAT3 Point3CubieSplineInterpolate(const std::vector<SControlPoint3>& controlPoint1s, float x);

public:
	static FPoint1Interpolater GetPoint1Interpolater(EInterpolationMethod interpolationMethod);
	static FPoint2Interpolater GetPoint2Interpolater(EInterpolationMethod interpolationMethod);
	static FPoint3Interpolater GetPoint3Interpolater(EInterpolationMethod interpolationMethod);
};

