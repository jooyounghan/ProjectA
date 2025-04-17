#include "LinearInterpolater.h"
#include <exception>

using namespace std;
using namespace DirectX;

LinearInterpolater::LinearInterpolater(
	const SControlPoint& startPoint, 
	const SControlPoint& endPoint, 
	const vector<SControlPoint>& controlPoints
)
	: AInterpolater<XMFLOAT2>(startPoint, endPoint, controlPoints)
{
	// 최소 2개의 N개 Control Point 확보
	vector<SControlPoint> linearContorlPoints = GetControlPoints(startPoint, endPoint, controlPoints);

    const size_t lineControlPointsStepCount = linearContorlPoints.size() - 1;
    for (size_t idx = 0; idx < lineControlPointsStepCount; ++idx)
    {
        const SControlPoint& point1 = linearContorlPoints[idx];
        const SControlPoint& point2 = linearContorlPoints[idx + 1];

        XMFLOAT2 coefficient;
        coefficient.x = (point1.y - point2.y) / (point1.x - point2.x);
        coefficient.y = (point1.y + point2.y - coefficient.x * (point1.x + point2.x)) / 2.f;
        coefficients.emplace_back(coefficient);
    }
}

float LinearInterpolater::GetInterpolated(float x) noexcept
{
    size_t coefficientIndex = GetCoefficientIndex(x);
    XMFLOAT2 coefficient = coefficients[coefficientIndex];
    return coefficient.x * x + coefficient.y;
}