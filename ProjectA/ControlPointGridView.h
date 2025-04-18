#pragma once
#include "Interpolater.h"
#include "imgui.h"

#include <string>
#include <format>
#include <algorithm>

class ControlPointGridView
{
public:
	template<uint32_t Dim>
	static bool HandleControlPointsGridView(
		const std::string& xValueName,
		const std::array<std::string, Dim>& yValueNames,
		const std::string& controlPointsName,
		float yStep, float yMin, float yMax,
		SControlPoint<Dim>& startPoint,
		SControlPoint<Dim>& endPoint,
		std::vector<SControlPoint<Dim>>& controlPoints
	);
};

template<uint32_t Dim>
inline bool ControlPointGridView::HandleControlPointsGridView(
	const std::string& xValueName, 
	const std::array<std::string, Dim>& yValueNames, 
	const std::string& controlPointsName, 
	float yStep, float yMin, float yMax, 
	SControlPoint<Dim>& startPoint, 
	SControlPoint<Dim>& endPoint, 
	std::vector<SControlPoint<Dim>>& controlPoints
)
{
	bool isChanged = false;

	std::string controlPointID = std::format("{} 추가", controlPointsName);
	std::string controlAddPopup = std::format("{} 추가 버튼", controlPointsName);
	std::string controlAddButton = std::format("+##{}", controlPointsName);
	ImGui::Text(controlPointID.c_str());
	ImGui::SameLine();

	if (ImGui::SmallButton(controlAddButton.c_str()))
	{
		ImGui::OpenPopup(controlAddPopup.c_str());
	}

	bool addControlPointsFlag = true;
	if (ImGui::BeginPopupModal(controlAddPopup.c_str(), &addControlPointsFlag))
	{
		static float x = 0.f;
		static std::array<float, Dim> y = MakeZeroArray<Dim>();

		x = std::min(std::max(startPoint.x, x), endPoint.x);
		ImGui::DragFloat(xValueName.c_str(), &x, 0.1f, startPoint.x, endPoint.x, "%.1f");

		// ========================================================================
		if (Dim == 3)
		{
			ImGui::ColorPicker3("색상 선택", y.data());
		}
		else
		{
			for (uint32_t dimension = 0; dimension < Dim; ++dimension)
			{
				ImGui::DragFloat(yValueNames[dimension].c_str(), &y[dimension], yStep, yMin, yMax, "%.2f");
			}
		}
		// ===========================================================================

		if (isChanged |= ImGui::Button("추가"))
		{
			controlPoints.emplace_back(x, y);
			std::sort(
				controlPoints.begin(), 
				controlPoints.end(), 
				[](const SControlPoint<Dim>& a, const SControlPoint<Dim>& b) 
				{
					return a.x < b.x;
				}
			);
			ImGui::CloseCurrentPopup();
		}
		ImGui::SameLine();
		if (ImGui::Button("취소"))
		{
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}

	if (ImGui::BeginTable(controlPointsName.c_str(), Dim == 3 ?  3 : 1 + Dim + 1, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
	{
		ImGui::TableSetupColumn(xValueName.c_str());

		// ======================================================================================
		if (Dim == 3)
		{
			ImGui::TableSetupColumn("RGB 색상 값");
		}
		else
		{
			for (uint32_t dimension = 0; dimension < Dim; ++dimension)
			{
				ImGui::TableSetupColumn(yValueNames[dimension].c_str());
			}
		}
		// ======================================================================================

		ImGui::TableSetupColumn("제거");
		ImGui::TableHeadersRow();

		std::string pointIn = std::format("{:.2f}", startPoint.x);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::TextUnformatted(pointIn.c_str());

		// ======================================================================================
		if (Dim == 3)
		{
			ImGui::TableSetColumnIndex(1);
			ImGui::ColorEdit3("##StartPoint", startPoint.y.data());
		}
		else
		{
			for (uint32_t dimension = 0; dimension < Dim; ++dimension)
			{
				ImGui::TableSetColumnIndex(dimension + 1);
				isChanged |= ImGui::DragFloat(std::format("##StartPoint{}", dimension).c_str(), &startPoint.y[dimension], yStep, yMin, yMax, "%.2f");
			}
		}
		// ======================================================================================

		uint32_t removeIdx = ~0;
		uint32_t controlPointsCount = static_cast<uint32_t>(controlPoints.size());
		for (uint32_t idx = 0; idx < controlPointsCount; ++idx)
		{
			SControlPoint<Dim>& controlPoint = controlPoints[idx];
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			pointIn = std::format("{:.2f}", controlPoint.x);
			ImGui::TextUnformatted(pointIn.c_str());
			
			// ======================================================================================
			if (Dim == 3)
			{
				ImGui::TableSetColumnIndex(1);
				ImGui::ColorEdit3(std::format("##MidPoint{}", idx).c_str(), controlPoint.y.data());
			}
			else
			{
				for (uint32_t dimension = 0; dimension < Dim; ++dimension)
				{
					ImGui::TableSetColumnIndex(dimension + 1);
					isChanged |= ImGui::DragFloat(std::format("##MidPoint{}{}", idx, dimension).c_str(), &controlPoint.y[dimension], yStep, yMin, yMax, "%.2f");
				}
			}

			// ======================================================================================

			// ======================================================================================
			ImGui::TableSetColumnIndex(Dim == 3 ? 2 : Dim + 1);
			// ======================================================================================

			ImGui::PushID(std::format("##삭제{}", idx).c_str());
			if (ImGui::SmallButton("삭제"))
			{
				removeIdx = idx;
			}
			ImGui::PopID();
		}

		if (removeIdx < controlPointsCount)
		{
			isChanged = true;
			controlPoints.erase(controlPoints.begin() + removeIdx);
		}

		pointIn = std::format("{:.2f}", endPoint.x);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::TextUnformatted(pointIn.c_str());
		// ======================================================================================
		if (Dim == 3)
		{
			ImGui::TableSetColumnIndex(1);
			ImGui::ColorEdit3("##EndPoint", endPoint.y.data());
		}
		else
		{
			for (uint32_t dimension = 0; dimension < Dim; ++dimension)
			{
				ImGui::TableSetColumnIndex(dimension + 1);
				isChanged |= ImGui::DragFloat(std::format("##EndPoint{}", dimension).c_str(), &endPoint.y[dimension], yStep, yMin, yMax, "%.2f");
			}
		}
		// ======================================================================================
		ImGui::EndTable();
	}
	return isChanged;
}
