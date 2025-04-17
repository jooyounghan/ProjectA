#pragma once
#include "Interpolater.h"

class LinearInterpolater : public AInterpolater<DirectX::XMFLOAT2>
{
public:
	LinearInterpolater(
		const SControlPoint& startPoint,
		const SControlPoint& endPoint,
		const std::vector<SControlPoint>& controlPoints
	);
	~LinearInterpolater() override = default;

public:
	virtual float GetInterpolated(float x) noexcept override;
};
