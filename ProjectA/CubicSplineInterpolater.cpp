#include "CubicSplineInterpolater.h"

using namespace std;
using namespace DirectX;

CubicSplineInterpolater::CubicSplineInterpolater(
	const SControlPoint& startPoint, 
	const SControlPoint& endPoint, 
	const vector<SControlPoint>& controlPoints
)
	: AInterpolater<XMVECTOR>(startPoint, endPoint, controlPoints)
{
	// 최소 4개의 Control Point 확보
	vector<SControlPoint> cubicSplineContorlPoints = GetControlPoints(startPoint, endPoint, controlPoints);

    const size_t cubicSplineContorlPointsStepCount = cubicSplineContorlPoints.size() - 3;
    for (size_t idx = 0; idx < cubicSplineContorlPointsStepCount; ++idx)
    {
        const float y0 = cubicSplineContorlPoints[idx].y;
        const float y1 = cubicSplineContorlPoints[idx + 1].y;
        const float y2 = cubicSplineContorlPoints[idx + 2].y;
        const float y3 = cubicSplineContorlPoints[idx + 3].y;

        XMVECTOR coefficient = XMVectorSet(
            0.5f * (-y0 + 3.0f * y1 - 3.0f * y2 + y3),
            0.5f * (2.0f * y0 - 5.0f * y1 + 4.0f * y2 - y3),
            0.5f * (-y0 + y2),
            0.5f * (2.0f * y1)
        );

        coefficients.emplace_back(coefficient);
    }
}

vector<SControlPoint> CubicSplineInterpolater::GetControlPoints(
	const SControlPoint& startPoint, 
	const SControlPoint& endPoint,
	const vector<SControlPoint>& controlPoints
)
{
	vector<SControlPoint> cubicSplineContorlPoints = AInterpolater::GetControlPoints(startPoint, endPoint, controlPoints);
	cubicSplineContorlPoints.insert(cubicSplineContorlPoints.begin(), SControlPoint{ startPoint.x - 1.f, startPoint.y });
	cubicSplineContorlPoints.emplace_back(SControlPoint{ endPoint.x + 1.f, endPoint.y });
	return cubicSplineContorlPoints;
}

float CubicSplineInterpolater::GetInterpolated(float x) noexcept
{
	size_t coefficientIndex = GetCoefficientIndex(x);

	float x1 = xProfiles[coefficientIndex];
	float x2 = xProfiles[coefficientIndex + 1];
	float t = (x - x1) / (x2 - x1);

	XMVECTOR coefficient = coefficients[coefficientIndex];
	float a = XMVectorGetX(coefficient);
	float b = XMVectorGetY(coefficient);
	float c = XMVectorGetZ(coefficient);
	float d = XMVectorGetW(coefficient);
	return ((a * t + b) * t + c) * t + d;
}
