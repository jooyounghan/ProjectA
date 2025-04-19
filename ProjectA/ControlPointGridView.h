#pragma once
#include "Interpolater.h"
#include "imgui.h"

#include <sstream>
#include <string>
#include <format>
#include <algorithm>

template<uint32_t Dim>
class ControlPointGridView
{
public:
	ControlPointGridView(
		const std::string& xValueName,
		const std::array<std::string, Dim>& yValueNames,
		const std::string& controlPointsName,
		float yStep, float yMin, float yMax,
		SControlPoint<Dim>& startPoint,
		SControlPoint<Dim>& endPoint,
		std::vector<SControlPoint<Dim>>& controlPoints,
		bool isIncremental = false
	);

protected:
	std::string m_xValueName;
	std::array<std::string, Dim> m_yValueNames;
	std::string m_yValueName;
	std::string m_controlPointsName;
	float m_yStep;
	float m_yMin; 
	float m_yMax;
	bool m_isIncremental = false;

protected:
	SControlPoint<Dim>& m_startPoint;
	SControlPoint<Dim>& m_endPoint;
	std::vector<SControlPoint<Dim>>& m_controlPoints;

public:
	bool DrawControlPointGridView();

private:
	void HandleNewControlPoint(float& x, std::array<float, Dim>& y);

private:
	int GetTotalColumnCount();
	int GetDeleteButtonColumnIndex();
	void SetColumns();
	void DrawControlPointTable(bool& isChanged);
	void HandleRowControlPoint(
		uint32_t columnOffset, 
		SControlPoint<Dim>& controlPoint, 
		bool& isChanged
	);
};

template<uint32_t Dim>
inline ControlPointGridView<Dim>::ControlPointGridView(
	const std::string& xValueName,
	const std::array<std::string, Dim>& yValueNames,
	const std::string& controlPointsName,
	float yStep, float yMin, float yMax,
	SControlPoint<Dim>& startPoint,
	SControlPoint<Dim>& endPoint,
	std::vector<SControlPoint<Dim>>& controlPoints,
	bool isIncremental
)
	: m_xValueName(xValueName), m_yValueNames(yValueNames),
	m_controlPointsName(controlPointsName),
	m_yStep(yStep), m_yMin(yMin), m_yMax(yMax),
	m_startPoint(startPoint),
	m_endPoint(endPoint),
	m_controlPoints(controlPoints),
	m_isIncremental(isIncremental)
{
	std::string delimiter = " / ";
	std::ostringstream oss;
	for (uint32_t dimension = 0; dimension < Dim; ++dimension)
	{
		if (dimension > 0)
			oss << delimiter;
		oss << m_yValueNames[dimension];
	}
	m_yValueName = oss.str();
}


template<uint32_t Dim>
inline bool ControlPointGridView<Dim>::DrawControlPointGridView()
{
	bool isChanged = false;
	
	std::string controlPointID = std::format("{} 추가", m_controlPointsName);
	std::string controlAddPopup = std::format("{} 추가 버튼", m_controlPointsName);
	std::string controlAddButton = std::format("+##{}", m_controlPointsName);
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
		x = std::min(std::max(m_startPoint.x, x), m_endPoint.x);
	
		HandleNewControlPoint(x, y);
	
		if (isChanged |= ImGui::Button("추가"))
		{
			m_controlPoints.emplace_back(x, y);
			std::sort(
				m_controlPoints.begin(),
				m_controlPoints.end(),
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
	
	DrawControlPointTable(isChanged);
	return isChanged;
}


template<uint32_t Dim>
inline void ControlPointGridView<Dim>::HandleNewControlPoint(float& x, std::array<float, Dim>& y)
{
	ImGui::SeparatorText("X 값");
	ImGui::DragFloat(m_xValueName.c_str(), &x, 0.1f, m_startPoint.x, m_endPoint.x, "%.1f");

	ImGui::SeparatorText("Y 값");
	for (uint32_t dimension = 0; dimension < Dim; ++dimension)
	{
		if (ImGui::DragFloat(
			m_yValueNames[dimension].c_str(), &y[dimension], m_yStep,
			(m_isIncremental && dimension > 0) ? y[dimension - 1] : m_yMin,
			(m_isIncremental && dimension < (Dim - 1)) ? y[dimension + 1] : m_yMax,
			"%.2f"
		))
		{
			if (m_isIncremental)
			{
				for (uint32_t currentDim = dimension; currentDim < (Dim - 1); ++currentDim)
				{
					y[currentDim + 1] = std::max(y[currentDim], y[currentDim + 1]);
				}
			}
		}
	}
}

template<>
inline void ControlPointGridView<3>::HandleNewControlPoint(float& x, std::array<float, 3>& y)
{
	ImGui::SeparatorText("X 값");
	ImGui::DragFloat(m_xValueName.c_str(), &x, 0.1f, m_startPoint.x, m_endPoint.x, "%.1f");

	ImGui::SeparatorText("Y 값");
	ImGui::ColorPicker3(m_yValueName.c_str(), y.data());
}

template<uint32_t Dim>
inline int ControlPointGridView<Dim>::GetTotalColumnCount()
{
	return 1 + Dim + 1;
}

template<uint32_t Dim>
inline int ControlPointGridView<Dim>::GetDeleteButtonColumnIndex()
{
	return Dim + 1;
}


template<>
inline int ControlPointGridView<3>::GetTotalColumnCount()
{
	return 3;
}

template<>
inline int ControlPointGridView<3>::GetDeleteButtonColumnIndex()
{
	return 2;
}

template<uint32_t Dim>
inline void ControlPointGridView<Dim>::SetColumns()
{
	ImGui::TableSetupColumn(m_xValueName.c_str());
	for (uint32_t dimension = 0; dimension < Dim; ++dimension)
	{
		ImGui::TableSetupColumn(m_yValueNames[dimension].c_str());
	}
	ImGui::TableSetupColumn("제거");
}

template<>
inline void ControlPointGridView<3>::SetColumns()
{
	ImGui::TableSetupColumn(m_xValueName.c_str());
	ImGui::TableSetupColumn(m_yValueName.c_str());
	ImGui::TableSetupColumn("제거");
}

template<uint32_t Dim>
inline void ControlPointGridView<Dim>::DrawControlPointTable(bool& isChanged)
{
	if (ImGui::BeginTable(m_controlPointsName.c_str(), GetTotalColumnCount(), ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg))
	{
		SetColumns();
		
		ImGui::TableHeadersRow();

		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		std::string pointIn = std::format("{:.2f}", m_startPoint.x);
		ImGui::TextUnformatted(pointIn.c_str());

		HandleRowControlPoint(1, m_startPoint, isChanged);

		uint32_t removeIdx = ~0;
		uint32_t controlPointsCount = static_cast<uint32_t>(m_controlPoints.size());
		for (uint32_t idx = 0; idx < controlPointsCount; ++idx)
		{
			SControlPoint<Dim>& controlPoint = m_controlPoints[idx];
			ImGui::TableNextRow();
			ImGui::TableSetColumnIndex(0);
			pointIn = std::format("{:.2f}", controlPoint.x);
			ImGui::TextUnformatted(pointIn.c_str());

			HandleRowControlPoint(1, controlPoint, isChanged);

			ImGui::TableSetColumnIndex(GetDeleteButtonColumnIndex());

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
			m_controlPoints.erase(m_controlPoints.begin() + removeIdx);
		}

		pointIn = std::format("{:.2f}", m_endPoint.x);
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::TextUnformatted(pointIn.c_str());
		HandleRowControlPoint(1, m_endPoint, isChanged);
	
		ImGui::EndTable();
	}
}

template<uint32_t Dim>
inline void ControlPointGridView<Dim>::HandleRowControlPoint(
	uint32_t columnOffset,
	SControlPoint<Dim>& controlPoint,
	bool& isChanged
)
{
	for (uint32_t dimension = 0; dimension < Dim; ++dimension)
	{
		ImGui::TableSetColumnIndex(dimension + columnOffset);
		if (isChanged |= ImGui::DragFloat(
			std::format("##{}{}", size_t(&controlPoint), dimension).c_str(),
			&controlPoint.y[dimension],
			m_yStep,
			(m_isIncremental && dimension > 0) ? controlPoint.y[dimension - 1] : m_yMin,
			(m_isIncremental && dimension < (Dim - 1)) ? controlPoint.y[dimension + 1] : m_yMax,
			"%.2f"
		))
		{
			if (m_isIncremental)
			{
				for (uint32_t currentDim = dimension; currentDim < (Dim - 1); ++currentDim)
				{
					controlPoint.y[currentDim + 1] = std::max(controlPoint.y[currentDim], controlPoint.y[currentDim + 1]);
				}
			}
		}
	}
}


template<>
inline void ControlPointGridView<3>::HandleRowControlPoint(
	uint32_t columnOffset,
	SControlPoint<3>& controlPoint,
	bool& isChanged
)
{
	ImGui::TableSetColumnIndex(columnOffset);
	isChanged |= ImGui::ColorEdit3(std::format("##{}", size_t(&controlPoint)).c_str(), controlPoint.y.data());
}
