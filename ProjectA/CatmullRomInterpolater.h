#pragma once
#include "Interpolater.h"

template<uint32_t Dim, bool GPUInterpolateOn>
class CCatmullRomInterpolater : public AInterpolater<Dim, 4, GPUInterpolateOn>
{
public:
	CCatmullRomInterpolater(
		const SControlPoint<Dim>& startPoint,
		const SControlPoint<Dim>& endPoint,
		const std::vector<SControlPoint<Dim>>& controlPoints
	);
	~CCatmullRomInterpolater() override = default;

protected:
	using Parent = AInterpolater<Dim, 4, GPUInterpolateOn>;

public:
	virtual UINT GetInterpolaterFlag() override;
	virtual void UpdateCoefficient() override;
	virtual std::array<float, Dim> GetInterpolated(float x) noexcept override;

protected:
	virtual std::vector<SControlPoint<Dim>> GetControlPoints() override;
};

template<uint32_t Dim, bool GPUInterpolateOn>
inline CCatmullRomInterpolater<Dim, GPUInterpolateOn>::CCatmullRomInterpolater(
	const SControlPoint<Dim>& startPoint, 
	const SControlPoint<Dim>& endPoint, 
	const std::vector<SControlPoint<Dim>>& controlPoints
)
	: AInterpolater<Dim, 4, GPUInterpolateOn>(startPoint, endPoint, controlPoints)
{
	UpdateCoefficient();
}

template<uint32_t Dim, bool GPUInterpolateOn>
UINT CCatmullRomInterpolater<Dim, GPUInterpolateOn>::GetInterpolaterFlag()
{
	return 2;
}

template<uint32_t Dim, bool GPUInterpolateOn>
inline void CCatmullRomInterpolater<Dim, GPUInterpolateOn>::UpdateCoefficient()
{
	Parent::UpdateCoefficient();

	// 최소 4개의 Control Point 확보
	Parent::m_coefficients.clear();

	std::vector<SControlPoint<Dim>> catmullRomContorlPoints = GetControlPoints();

	const size_t catmullRomContorlPointsStepCount = catmullRomContorlPoints.size() - 3;
	for (size_t idx = 0; idx < catmullRomContorlPointsStepCount; ++idx)
	{
		const std::array<float, Dim>& y0 = catmullRomContorlPoints[idx].y;
		const std::array<float, Dim>& y1 = catmullRomContorlPoints[idx + 1].y;
		const std::array<float, Dim>& y2 = catmullRomContorlPoints[idx + 2].y;
		const std::array<float, Dim>& y3 = catmullRomContorlPoints[idx + 3].y;

		std::array<float, 4 * Dim> coefficient;
		for (uint32_t dimension = 0; dimension < Dim; ++dimension)
		{
			coefficient[4 * dimension + 0] = 0.5f * (-y0[dimension] + 3.0f * y1[dimension] - 3.0f * y2[dimension] + y3[dimension]);
			coefficient[4 * dimension + 1] = 0.5f * (2.0f * y0[dimension] - 5.0f * y1[dimension] + 4.0f * y2[dimension] - y3[dimension]);
			coefficient[4 * dimension + 2] = 0.5f * (-y0[dimension] + y2[dimension]);
			coefficient[4 * dimension + 3] = 0.5f * (2.0f * y1[dimension]);
		}
		Parent::m_coefficients.emplace_back(coefficient);
	}

	if (GPUInterpolateOn)
	{
		Parent::UpdateInterpolaterProperty();
	}
}

template<uint32_t Dim, bool GPUInterpolateOn>
inline std::array<float, Dim> CCatmullRomInterpolater<Dim, GPUInterpolateOn>::GetInterpolated(float x) noexcept
{
	size_t coefficientIndex = Parent::GetCoefficientIndex(x);
	
	float x1 = Parent::m_xProfiles[coefficientIndex];
	float x2 = Parent::m_xProfiles[coefficientIndex + 1];
	float t = (x - x1) / (x2 - x1);
	
	const std::array<float, 4 * Dim>& coefficient = Parent::m_coefficients[coefficientIndex];
	
	std::array<float, Dim> result;
	for (uint32_t dimension = 0; dimension < Dim; ++dimension)
	{
		float a = coefficient[4 * dimension + 0];
		float b = coefficient[4 * dimension + 1];
		float c = coefficient[4 * dimension + 2];
		float d = coefficient[4 * dimension + 3];
		result[dimension] = ((a * t + b) * t + c) * t + d;
	}
	return result;
}

template<uint32_t Dim, bool GPUInterpolateOn>
inline std::vector<SControlPoint<Dim>> CCatmullRomInterpolater<Dim, GPUInterpolateOn>::GetControlPoints()
{
	std::vector<SControlPoint<Dim>> catmullRomContorlPoints = Parent::GetControlPoints();
	catmullRomContorlPoints.insert(catmullRomContorlPoints.begin(), SControlPoint<Dim>{ Parent::m_startPoint.x - 1.f, Parent::m_startPoint.y });
	catmullRomContorlPoints.emplace_back(SControlPoint<Dim>{ Parent::m_endPoint.x + 1.f, Parent::m_endPoint.y });
	return catmullRomContorlPoints;
}
