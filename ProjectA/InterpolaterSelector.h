#pragma once
#include "BaseSelector.h"
#include "LinearInterpolater.h"
#include "CubicSplineInterpolater.h"
#include "CatmullRomInterpolater.h"

#include "imgui.h"
#include "implot.h"

#include <format>
#include <memory>

template<uint32_t dim, bool GPUInterpolateOn>
class InterpolaterSelectPlotter;

class InterpolaterSelectorHelper
{
template<uint32_t dim, bool GPUInterpolateOn>
friend class InterpolaterSelectPlotter;

protected:
	static std::unordered_map<EInterpolationMethod, std::string> GInterpolationMethodStringMap;
};

template<uint32_t Dim, bool GPUInterpolateOn>
class InterpolaterSelectPlotter : public BaseSelector<EInterpolationMethod>
{
	friend InterpolaterSelectorHelper;

public:
	InterpolaterSelectPlotter(
		const std::string& selectorName,
		const std::string& graphTitle,
		const std::array<std::string, Dim>& scatterLabels,
		const SControlPoint<Dim>& startPoint,
		const SControlPoint<Dim>& endPoint,
		const std::vector<SControlPoint<Dim>>& controlPoints
	);
	~InterpolaterSelectPlotter() override = default;

protected:
	std::string m_graphTitle;
	std::array<std::string, Dim> m_scatterLabels;
	const SControlPoint<Dim>& m_startPoint;
	const SControlPoint<Dim>& m_endPoint;
	const std::vector<SControlPoint<Dim>>& m_controlPoints;

protected:
	std::vector<float> m_pointIns;
	std::array<std::vector<float>, Dim> m_pointOuts;
	std::vector<float> m_timeSteps;
	std::array<std::vector<float>, Dim> m_interpolatedResults;


protected:
	IInterpolater<Dim, GPUInterpolateOn>* m_interpolater;

public:
	void SetInterpolater(
		EInterpolationMethod interpolationMethod, 
		std::unique_ptr<IInterpolater<Dim, GPUInterpolateOn>>& interpolater
	);

public:
	void RedrawSelectPlotter();
	void ViewInterpolatedPlots();

protected:
	void UpdateInterpolatedResult();
	void UpdateTimeSteps(size_t numSteps);
};



template<uint32_t Dim, bool GPUInterpolateOn>
InterpolaterSelectPlotter<Dim, GPUInterpolateOn>::InterpolaterSelectPlotter(
	const std::string& selectorName, 
	const std::string& graphTitle, 
	const std::array<std::string, Dim>& scatterLabels,
	const SControlPoint<Dim>& startPoint, 
	const SControlPoint<Dim>& endPoint, 
	const std::vector<SControlPoint<Dim>>& controlPoints
)
	: BaseSelector<EInterpolationMethod>(selectorName, InterpolaterSelectorHelper::GInterpolationMethodStringMap),
	m_graphTitle(graphTitle),
	m_scatterLabels(scatterLabels),
	m_startPoint(startPoint),
	m_endPoint(endPoint),
	m_controlPoints(controlPoints)
{
}

template<uint32_t Dim, bool GPUInterpolateOn>
void InterpolaterSelectPlotter<Dim, GPUInterpolateOn>::SetInterpolater(
	EInterpolationMethod interpolationMethod, 
	std::unique_ptr<IInterpolater<Dim, GPUInterpolateOn>>& interpolater
)
{
	if (interpolater)
	{
		interpolater.reset();
	}

	switch (interpolationMethod)
	{
	case EInterpolationMethod::Linear:
		interpolater = std::make_unique<LinearInterpolater<Dim, GPUInterpolateOn>>(m_startPoint, m_endPoint, m_controlPoints);
		break;
	case EInterpolationMethod::CubicSpline:
		interpolater = std::make_unique<CubicSplineInterpolater<Dim, GPUInterpolateOn>>(m_startPoint, m_endPoint, m_controlPoints);
		break;
	case EInterpolationMethod::CatmullRom:
		interpolater = std::make_unique<CatmullRomInterpolater<Dim, GPUInterpolateOn>>(m_startPoint, m_endPoint, m_controlPoints);
		break;
	}
	if (interpolater != nullptr)
	{
		m_interpolater = interpolater.get();
		UpdateInterpolatedResult();
	}
}

template<uint32_t Dim, bool GPUInterpolateOn>
inline void InterpolaterSelectPlotter<Dim, GPUInterpolateOn>::RedrawSelectPlotter()
{
	m_pointIns.clear();
	for (uint32_t dimension = 0; dimension < Dim; ++dimension)
	{
		m_pointOuts[dimension].clear();

	}

	m_pointIns.reserve(m_controlPoints.size() + 2);
	for (uint32_t dimension = 0; dimension < Dim; ++dimension)
	{
		m_pointOuts[dimension].reserve(m_controlPoints.size() + 2);

	}
	m_pointIns.emplace_back(m_startPoint.x);
	for (uint32_t dimension = 0; dimension < Dim; ++dimension)
	{
		m_pointOuts[dimension].emplace_back(m_startPoint.y[dimension]);
	}
	for (const auto& cp : m_controlPoints)
	{
		m_pointIns.emplace_back(cp.x);
		for (uint32_t dimension = 0; dimension < Dim; ++dimension)
		{
			m_pointOuts[dimension].emplace_back(cp.y[dimension]);
		}
	}
	m_pointIns.emplace_back(m_endPoint.x);
	for (uint32_t dimension = 0; dimension < Dim; ++dimension)
	{
		m_pointOuts[dimension].emplace_back(m_endPoint.y[dimension]);
	}

	UpdateTimeSteps(100);
	UpdateInterpolatedResult();
}


template<uint32_t Dim, bool GPUInterpolateOn>
void InterpolaterSelectPlotter<Dim, GPUInterpolateOn>::ViewInterpolatedPlots()
{
	if (ImPlot::BeginPlot(m_graphTitle.c_str(), ImVec2(-1, 0), ImPlotFlags_NoInputs))
	{
		ImPlot::SetupAxes(nullptr, nullptr, ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
		for (uint32_t dimension = 0; dimension < Dim; ++dimension)
		{
			ImPlot::PlotScatter(m_scatterLabels[dimension].c_str(), m_pointIns.data(), m_pointOuts[dimension].data(), static_cast<int>(m_pointIns.size()));
		}

		for (uint32_t dimension = 0; dimension < Dim; ++dimension)
		{
			ImPlot::PlotLine(m_scatterLabels[dimension].c_str(), m_timeSteps.data(), m_interpolatedResults[dimension].data(), static_cast<int>(m_timeSteps.size()));
		}
		ImPlot::EndPlot();
	}
}

template<uint32_t Dim, bool GPUInterpolateOn>
inline void InterpolaterSelectPlotter<Dim, GPUInterpolateOn>::UpdateInterpolatedResult()
{
	if (m_interpolater)
	{
		for (uint32_t dimension = 0; dimension < Dim; ++dimension)
		{
			m_interpolatedResults[dimension].clear();
		}

		for (auto& timeStep : m_timeSteps)
		{
			std::array<float, Dim> interpolatedResult = m_interpolater->GetInterpolated(timeStep);
			for (uint32_t dimension = 0; dimension < Dim; ++dimension)
			{
				m_interpolatedResults[dimension].emplace_back(interpolatedResult[dimension]);
			}
		}
	}
}

template<uint32_t Dim, bool GPUInterpolateOn>
inline void InterpolaterSelectPlotter<Dim, GPUInterpolateOn>::UpdateTimeSteps(size_t numSteps)
{
	m_timeSteps.clear();
	float start = m_startPoint.x;
	float end = m_endPoint.x;

	float step = (end - start) / static_cast<float>(numSteps);
	m_timeSteps.reserve(numSteps + 1);
	for (size_t i = 0; i <= numSteps; ++i)
	{
		m_timeSteps.emplace_back(start + step * i);
	}
}