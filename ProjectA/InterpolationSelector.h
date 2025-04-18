#pragma once
#include "BaseSelector.h"
#include "LinearInterpolater.h"
#include "CubicSplineInterpolater.h"

#include "imgui.h"
#include "implot.h"

#include <format>
#include <memory>

enum class EInterpolationMethod
{
	Linear,
	CubicSpline
};

class InterpolationSelector : public BaseSelector<EInterpolationMethod>
{
public:
	static std::unordered_map<EInterpolationMethod, std::string> GInterpolationMethodStringMap;

	template<uint32_t Dim>
	static std::unique_ptr<IInterpolater<Dim>> CreateInterpolater(
		EInterpolationMethod interpolationMethod,
		const SControlPoint<Dim>& startPoint,
		const SControlPoint<Dim>& endPoint,
		const std::vector<SControlPoint<Dim>>& controlPoints
	);

public:
	template<uint32_t Dim>
	static void ViewInterpolatedPoints(
		IInterpolater<Dim>* interpolater,
		const std::string& graphTitle,
		const std::array<std::string, Dim>& scatterLabels,
		const SControlPoint<Dim>& startPoint,
		const SControlPoint<Dim>& endPoint,
		const std::vector<SControlPoint<Dim>>& controlPoints
	);

private:
	static std::vector<float> MakeUniformStepsFromX(const std::vector<float>& xs, size_t numSteps);
};

template<uint32_t Dim>
inline std::unique_ptr<IInterpolater<Dim>> InterpolationSelector::CreateInterpolater(
	EInterpolationMethod interpolationMethod, 
	const SControlPoint<Dim>& startPoint, 
	const SControlPoint<Dim>& endPoint, 
	const std::vector<SControlPoint<Dim>>& controlPoints
)
{
	switch (interpolationMethod)
	{
	case EInterpolationMethod::Linear:
		return std::make_unique<LinearInterpolater<Dim>>(startPoint, endPoint, controlPoints);
		break;
	case EInterpolationMethod::CubicSpline:
		return std::make_unique<CubicSplineInterpolater<Dim>>(startPoint, endPoint, controlPoints);
		break;
	}
	return nullptr;
}

template<uint32_t Dim>
inline void InterpolationSelector::ViewInterpolatedPoints(
	IInterpolater<Dim>* interpolater, 
	const std::string& graphTitle, 
	const std::array<std::string, Dim>& scatterLabels,
	const SControlPoint<Dim>& startPoint, 
	const SControlPoint<Dim>& endPoint, 
	const std::vector<SControlPoint<Dim>>& controlPoints
)
{
	std::vector<float> pointIns;
	std::array<std::vector<float>, Dim> pointOuts;

#pragma region PointIns/PointOuts Á¤¸®
	pointIns.reserve(controlPoints.size() + 2);
	for (uint32_t dimension = 0; dimension < Dim; ++dimension)
	{
		pointOuts[dimension].reserve(controlPoints.size() + 2);

	}
	pointIns.emplace_back(startPoint.x);
	for (uint32_t dimension = 0; dimension < Dim; ++dimension)
	{
		pointOuts[dimension].emplace_back(startPoint.y[dimension]);
	}
	for (const auto& cp : controlPoints) 
	{
		pointIns.emplace_back(cp.x);
		for (uint32_t dimension = 0; dimension < Dim; ++dimension)
		{
			pointOuts[dimension].emplace_back(cp.y[dimension]);
		}
	}
	pointIns.emplace_back(endPoint.x);
	for (uint32_t dimension = 0; dimension < Dim; ++dimension)
	{
		pointOuts[dimension].emplace_back(endPoint.y[dimension]);
	}
#pragma endregion

	if (ImPlot::BeginPlot(graphTitle.c_str(), ImVec2(-1, 0), ImPlotFlags_NoInputs))
	{
		ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
		for (uint32_t dimension = 0; dimension < Dim; ++dimension)
		{
			ImPlot::PlotScatter(scatterLabels[dimension].c_str(), pointIns.data(), pointOuts[dimension].data(), static_cast<int>(pointIns.size()));
		}

		if (interpolater)
		{
			std::vector<float> timeSteps = MakeUniformStepsFromX(pointIns, 100);
			std::array<std::vector<float>, Dim> interpolatedResults;
			for (auto& timeStep : timeSteps)
			{
				std::array<float, Dim> interpolatedResult = interpolater->GetInterpolated(timeStep);
				for (uint32_t dimension = 0; dimension < Dim; ++dimension)
				{
					interpolatedResults[dimension].emplace_back(interpolatedResult[dimension]);
				}
			}
			for (uint32_t dimension = 0; dimension < Dim; ++dimension)
			{
				ImPlot::PlotLine(scatterLabels[dimension].c_str(), timeSteps.data(), interpolatedResults[dimension].data(), 100);
			}
		}
		ImPlot::EndPlot();
	}

}
