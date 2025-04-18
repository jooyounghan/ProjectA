#include "ControlPointGridView.h"
#include "imgui.h"

#include <format>
#include <algorithm>

using namespace std;
using namespace DirectX;
using namespace ImGui;

bool ControlPointGridView::HandleControlPointsGridView(
	const string& xValueName,
	const string& yValueName, 
	const string& controlPointsName,
	float yStep, float yMin, float yMax,
	SControlPoint& startPoint, 
	SControlPoint& endPoint, 
	vector<SControlPoint>& controlPoints
)
{
	bool isChanged = false;

	Text("Control Point 추가");
	SameLine();
	if (ImGui::SmallButton("+"))
	{
		OpenPopup("AddControlPoints");
	}

	bool addControlPointsFlag = true;
	if (ImGui::BeginPopupModal("AddControlPoints", &addControlPointsFlag))
	{
		static float x = 0.f;
		static float y = 0.f;

		x = min(max(startPoint.x, x), endPoint.x);
		DragFloat(xValueName.c_str(), &x, 0.1f, startPoint.x, endPoint.x, "%.1f");
		DragFloat(yValueName.c_str(), &y, yStep, yMin, yMax, "%.2f");
		if (isChanged |= ImGui::Button("추가"))
		{
			controlPoints.emplace_back(x, y);
			std::sort(controlPoints.begin(), controlPoints.end(), [](const SControlPoint& a, const SControlPoint& b) {
				return a.x < b.x;
			});
			ImGui::CloseCurrentPopup();
		}
		SameLine();
		if (ImGui::Button("취소"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (BeginTable(controlPointsName.c_str(), 3, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
	{
		TableSetupColumn(xValueName.c_str());
		TableSetupColumn(yValueName.c_str());
		TableSetupColumn("제거");
		TableHeadersRow();

		string pointIn = format("{:.2f}", startPoint.x);
		TableNextRow();
		TableSetColumnIndex(0);
		TextUnformatted(pointIn.c_str());
		TableSetColumnIndex(1);
		isChanged |= DragFloat("##StartPoint", &startPoint.y, yStep, yMin, yMax, "%.2f");

		uint32_t removeIdx = ~0;
		uint32_t controlPointsCount = static_cast<uint32_t>(controlPoints.size());
		for (uint32_t idx = 0; idx < controlPointsCount; ++idx)
		{
			SControlPoint& controlPoint = controlPoints[idx];
			pointIn = format("{:.2f}", controlPoint.x);
			string pointOut = format("{:.2f}", controlPoint.y);
			TableNextRow();
			TableSetColumnIndex(0);
			TextUnformatted(pointIn.c_str());
			TableSetColumnIndex(1);
			TextUnformatted(pointOut.c_str());
			TableSetColumnIndex(2);
			if (ImGui::SmallButton("삭제"))
			{
				removeIdx = idx;
			}
		}

		if (removeIdx < controlPointsCount)
		{
			isChanged = true;
			controlPoints.erase(controlPoints.begin() + removeIdx);
		}

		pointIn = format("{:.2f}", endPoint.x);
		TableNextRow();
		TableSetColumnIndex(0);
		TextUnformatted(pointIn.c_str());
		TableSetColumnIndex(1);
		isChanged |= DragFloat("##EndPoint", &endPoint.y, yStep, yMin, yMax, "%.2f");

		EndTable();
	}


	return isChanged;
}
