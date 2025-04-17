#pragma once
#include "Interpolater.h"

class CubicSplineInterpolater : public AInterpolater<DirectX::XMVECTOR>
{
public:
	CubicSplineInterpolater(
		const SControlPoint& startPoint,
		const SControlPoint& endPoint,
		const std::vector<SControlPoint>& controlPoints
	);
	~CubicSplineInterpolater() override = default;

protected:
	virtual std::vector<SControlPoint> GetControlPoints(
		const SControlPoint& startPoint,
		const SControlPoint& endPoint,
		const std::vector<SControlPoint>& controlPoints
	) override;

public:
	virtual float GetInterpolated(float x) noexcept override;
};
