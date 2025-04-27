#pragma once
#include "BaseSelector.h"
#include "LinearInterpolater.h"
#include "CubicSplineInterpolater.h"
#include "CatmullRomInterpolater.h"

#include "imgui.h"
#include "implot.h"

#include <format>
#include <memory>

template<uint32_t Dim>
class CInterpolaterSelectPlotter;

class InterpolaterSelectorHelper
{
template<uint32_t Dim>
friend class CInterpolaterSelectPlotter;

protected:
	static std::unordered_map<EInterpolationMethod, std::string> GInterpolationMethodStringMap;
};

template<uint32_t Dim>
class CInterpolaterSelectPlotter : public CBaseSelector<EInterpolationMethod>
{
	friend InterpolaterSelectorHelper;

public:
	CInterpolaterSelectPlotter(
		const std::string& selectorName,
		const std::string& graphTitle,
		const std::array<std::string, Dim>& scatterLabels,
		const SControlPoint<Dim>& startPoint,
		const SControlPoint<Dim>& endPoint,
		const std::vector<SControlPoint<Dim>>& controlPoints
	);
	~CInterpolaterSelectPlotter() override = default;

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
	IInterpolater<Dim>* m_interpolater;

public:
	void CreateInterpolater(
		CGPUInterpPropertyManager<Dim, 2>* d1GpuInterpProeprtyManager,
		CGPUInterpPropertyManager<Dim, 4>* d3GpuInterpProeprtyManager,
		EInterpolationMethod interpolationMethod, 
		std::unique_ptr<IInterpolater<Dim>>& interpolater
	);
	void ResetXYScale();
	void UpdateInterpolatedResult();
	void ViewInterpolatedPlots();

protected:
	void UpdateTimeSteps(size_t numSteps);
};



template<uint32_t Dim>
CInterpolaterSelectPlotter<Dim>::CInterpolaterSelectPlotter(
	const std::string& selectorName, 
	const std::string& graphTitle, 
	const std::array<std::string, Dim>& scatterLabels,
	const SControlPoint<Dim>& startPoint, 
	const SControlPoint<Dim>& endPoint, 
	const std::vector<SControlPoint<Dim>>& controlPoints
)
	: CBaseSelector<EInterpolationMethod>(selectorName, InterpolaterSelectorHelper::GInterpolationMethodStringMap),
	m_graphTitle(graphTitle),
	m_scatterLabels(scatterLabels),
	m_startPoint(startPoint),
	m_endPoint(endPoint),
	m_controlPoints(controlPoints)
{
}

template<uint32_t Dim>
void CInterpolaterSelectPlotter<Dim>::CreateInterpolater(
	CGPUInterpPropertyManager<Dim, 2>* d1GpuInterpProeprtyManager,
	CGPUInterpPropertyManager<Dim, 4>* d3GpuInterpProeprtyManager,
	EInterpolationMethod interpolationMethod, 
	std::unique_ptr<IInterpolater<Dim>>& interpolater
)
{
	if (interpolater)
	{
		interpolater.reset();
	}

	std::unique_ptr<AInterpolater<Dim, 2>> d1Interpolater;
	std::unique_ptr<AInterpolater<Dim, 4>> d3Interpolater;

	switch (interpolationMethod)
	{
	case EInterpolationMethod::Linear:
	{
		d1Interpolater = std::make_unique<CLinearInterpolater<Dim>>(m_startPoint, m_endPoint, m_controlPoints);
		d1Interpolater->SetGPUInterpolater(d1GpuInterpProeprtyManager);
		interpolater = std::move(d1Interpolater);
		break;
	}
	case EInterpolationMethod::CubicSpline:
		d3Interpolater = std::make_unique<CCubicSplineInterpolater<Dim>>(m_startPoint, m_endPoint, m_controlPoints);
		d3Interpolater->SetGPUInterpolater(d3GpuInterpProeprtyManager);
		interpolater = std::move(d3Interpolater);

		break;
	case EInterpolationMethod::CatmullRom:
		d3Interpolater = std::make_unique<CCatmullRomInterpolater<Dim>>(m_startPoint, m_endPoint, m_controlPoints);
		d3Interpolater->SetGPUInterpolater(d3GpuInterpProeprtyManager);
		interpolater = std::move(d3Interpolater);
		break;
	}
	if (interpolater != nullptr)
	{
		m_interpolater = interpolater.get();
		UpdateInterpolatedResult();
	}
}

template<uint32_t Dim>
inline void CInterpolaterSelectPlotter<Dim>::ResetXYScale()
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

template<uint32_t Dim>
inline void CInterpolaterSelectPlotter<Dim>::UpdateInterpolatedResult()
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

template<uint32_t Dim>
void CInterpolaterSelectPlotter<Dim>::ViewInterpolatedPlots()
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

template<uint32_t Dim>
inline void CInterpolaterSelectPlotter<Dim>::UpdateTimeSteps(size_t numSteps)
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