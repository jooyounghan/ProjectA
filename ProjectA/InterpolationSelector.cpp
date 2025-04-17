#include "InterpolationSelector.h"
#include "imgui.h"
#include "implot.h"

#include <format>

using namespace std;
using namespace DirectX;
using namespace ImGui;

unordered_map<EInterpolationMethod, string> InterpolationSelector::GInterpolationMethodStringMap
{
	{ EInterpolationMethod::None, "선택 안함" },
	{ EInterpolationMethod::Linear, "Linear" },
	{ EInterpolationMethod::CubicSpline, "Cubic Spline" }
};


void InterpolationSelector::GridViewControlPoint1s(
	const string& controlPointsName,
	const vector<SControlPoint>& controlPoints
)
{
	if (ImGui::BeginTable(controlPointsName.c_str(), 2, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
	{

		ImGui::TableSetupColumn("Time");
		ImGui::TableSetupColumn("Value");
		ImGui::TableHeadersRow();

		for (auto& controlPoint : controlPoints)
		{
			const string pointIn = format("{:.1f}", controlPoint.x);
			const string pointOut = format("{:.1f}", controlPoint.y);
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			ImGui::TextUnformatted(pointIn.c_str());
			ImGui::TableSetColumnIndex(1);
			ImGui::TextUnformatted(pointOut.c_str());
		}
		ImGui::EndTable();
	}
}

void InterpolationSelector::ViewInterpolatedPoints(
	AInterpolater* interpolater,
	const string& graphTitle,
	const string& scatterLabels,
	const string& lineLabels,
	const std::vector<SControlPoint>& controlPoints
)
{
	std::vector<float> pointIns;
	std::vector<float> pointOuts;

	pointIns.reserve(controlPoints.size());
	pointOuts.reserve(controlPoints.size());

	for (const auto& cp : controlPoints) 
	{
		pointIns.emplace_back(cp.x);
		pointOuts.emplace_back(cp.y);
	}

	if (ImPlot::BeginPlot(graphTitle.c_str(), ImVec2(-1, 0)))
	{
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

	if (xs.empty() || numSteps == 0)
	{
		throw exception("No X Array For Control Points");
		return result;
	}

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
