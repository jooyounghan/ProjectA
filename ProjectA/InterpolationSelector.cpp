#include "InterpolationSelector.h"
#include "imgui.h"
#include "implot.h"

#include <format>

using namespace std;
using namespace DirectX;
using namespace ImGui;

unordered_map<EInterpolationMethod, string> InterpolationSelector::GInterpolationMethodStringMap
{
	{ EInterpolationMethod::Linear, "선형 보간" },
	{ EInterpolationMethod::CubicSpline, "Cubic Spline 보간" }
};


std::unique_ptr<IInterpolater> InterpolationSelector::CreateInterpolater(
	EInterpolationMethod interpolationMethod, 
	const SControlPoint& startPoint, 
	const SControlPoint& endPoint, 
	const std::vector<SControlPoint>& controlPoints
)
{
	switch (interpolationMethod)
	{
		case EInterpolationMethod::Linear:
			return make_unique<LinearInterpolater>(startPoint, endPoint, controlPoints);
			break;
		case EInterpolationMethod::CubicSpline:
			return make_unique<CubicSplineInterpolater>(startPoint, endPoint, controlPoints);
			break;
	}
	return nullptr;
}

void InterpolationSelector::ViewInterpolatedPoints(
	IInterpolater* interpolater,
	const string& graphTitle,
	const string& scatterLabels,
	const string& lineLabels,
	const SControlPoint& startPoint,
	const SControlPoint& endPoint,
	const std::vector<SControlPoint>& controlPoints
)
{
	std::vector<float> pointIns;
	std::vector<float> pointOuts;

	pointIns.reserve(controlPoints.size() + 2);
	pointOuts.reserve(controlPoints.size() + 2);

	pointIns.emplace_back(startPoint.x);
	pointOuts.emplace_back(startPoint.y);
	for (const auto& cp : controlPoints) 
	{
		pointIns.emplace_back(cp.x);
		pointOuts.emplace_back(cp.y);
	}
	pointIns.emplace_back(endPoint.x);
	pointOuts.emplace_back(endPoint.y);

	if (ImPlot::BeginPlot(graphTitle.c_str(), ImVec2(-1, 0), ImPlotFlags_NoInputs))
	{
		ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
		ImPlot::PlotScatter(scatterLabels.c_str(), pointIns.data(), pointOuts.data(), static_cast<int>(controlPoints.size()));

		if (interpolater)
		{
			vector<float> timeSteps = MakeUniformStepsFromX(pointIns, 100);
			vector<float> interpolatedResults;
			for (auto& timeStep : timeSteps)
			{
				interpolatedResults.emplace_back(interpolater->GetInterpolated(timeStep));
			}
			ImPlot::PlotLine(lineLabels.c_str(), timeSteps.data(), interpolatedResults.data(), 100);
		}
		ImPlot::EndPlot();
	}
}

 vector<float> InterpolationSelector::MakeUniformStepsFromX(const vector<float>& xs, size_t numSteps)
{
	vector<float> result;

	float start = xs.front();
	float end = xs.back();
	float step = (end - start) / static_cast<float>(numSteps);

	result.reserve(numSteps + 1);
	for (size_t i = 0; i <= numSteps; ++i)
	{
		result.emplace_back(start + step * i);
	}

	return result;
}
