#pragma once
#include "Interpolater.h"
#include <string>

class ControlPointGridView
{
public:
	static bool HandleControlPointsGridView(
		const std::string& xValueName,
		const std::string& yValueName,
		const std::string& controlPointsName,
		float yStep, float yMin, float yMax,
		SControlPoint& startPoint,
		SControlPoint& endPoint,
		std::vector<SControlPoint>& controlPoints
	);
};

