#pragma once
#include <vector>
#include <functional>
#include <unordered_map>
#include <DirectXMath.h>
#include <memory>

enum class EInterpolationMethod
{
	None,
	Linear,
	CubicSpline
};


struct SControlPoint
{
	float x = 0.f;
	float y = 0.f;
};

class AInterpolater
{
public:
	AInterpolater() = default;
	virtual ~AInterpolater() = default;

protected:
	std::vector<float> xs;

public:
	virtual bool IsInterpolatable(size_t controlPointsCount) noexcept = 0;
	virtual bool GetCoefficients(const std::vector<SControlPoint>& controlPoints) noexcept;
	virtual float GetInterpolated(float x) noexcept = 0;

protected:
	virtual size_t GetIntervalIndex(float x) noexcept = 0;
};

class LinearInterpolater : public AInterpolater
{
public:
	LinearInterpolater() = default;
	~LinearInterpolater() override = default;

protected:
	std::vector<DirectX::XMFLOAT2> coefficients;

public:
	virtual bool IsInterpolatable(size_t controlPointsCount) noexcept override;
	virtual bool GetCoefficients(const std::vector<SControlPoint>& controlPoints) noexcept override;
	virtual float GetInterpolated(float x) noexcept override;

protected:
	virtual size_t GetIntervalIndex(float x) noexcept override;
};

class CubicSplineInterpolater : public AInterpolater
{
public:
	CubicSplineInterpolater() = default;
	~CubicSplineInterpolater() override = default;

protected:
	std::vector<DirectX::XMVECTOR> coefficients;

public:
	virtual bool IsInterpolatable(size_t controlPointsCount) noexcept override;
	virtual bool GetCoefficients(const std::vector<SControlPoint>& controlPoints) noexcept override;
	virtual float GetInterpolated(float x) noexcept override;

protected:
	virtual size_t GetIntervalIndex(float x) noexcept override;
};

class InterpolaterHelper
{
public:
	static std::unique_ptr<AInterpolater> GetInterpolater(EInterpolationMethod interpolationMethod);
};
