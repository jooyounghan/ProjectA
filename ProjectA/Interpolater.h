#pragma once
#include "InterpolaterStructure.h"

#include <vector>
#include <DirectXMath.h>
#include <concepts>

template <size_t Dim, size_t... Indices>
constexpr std::array<float, Dim> MakeZeroArrayImpl(std::index_sequence<Indices...>) 
{
	return { ((void)Indices, 0.f)... }; 
}

template <size_t Dim>
constexpr std::array<float, Dim> MakeZeroArray() {
	return MakeZeroArrayImpl<Dim>(std::make_index_sequence<Dim>{});
}

template <typename... Args>
constexpr auto MakeArray(Args... args)->std::array<float, sizeof...(Args)> 
{
	return { static_cast<float>(args)... };
}

template<typename T, uint32_t Dim>
concept IsControlPoint = requires(T t)
{
	{ t.x } -> std::same_as<float>;
	{ t.y } -> std::same_as<std::array<float, Dim>>;
};

template<uint32_t Dim>
class IInterpolater
{
public:
	virtual void UpdateCoefficient() = 0;
	virtual std::array<float, Dim> GetInterpolated(float x) noexcept = 0;
};

template<uint32_t Dim, uint32_t CoefficientCount>
class AInterpolater : public IInterpolater<Dim>
{
public:
	AInterpolater(
		const SControlPoint<Dim>& startPoint,
		const SControlPoint<Dim>& endPoint,
		const std::vector<SControlPoint<Dim>>& controlPoints
	);
	virtual ~AInterpolater() = default;

protected:
	const SControlPoint<Dim>& m_startPoint;
	const SControlPoint<Dim>& m_endPoint;
	const std::vector<SControlPoint<Dim>>& m_controlPoints;

protected:
	std::vector<float> m_xProfiles;
	using CoeffType = std::array<float, CoefficientCount * Dim>;
	std::vector<CoeffType> m_coefficients;

protected:
	virtual void UpdateCoefficient() override;
	virtual std::vector<SControlPoint<Dim>> GetControlPoints();

protected:
	size_t GetCoefficientIndex(float x) noexcept;
};

template<uint32_t Dim, uint32_t CoefficientCount>
inline AInterpolater<Dim, CoefficientCount>::AInterpolater(
	const SControlPoint<Dim>& startPoint, 
	const SControlPoint<Dim>& endPoint, 
	const std::vector<SControlPoint<Dim>>& controlPoints
)
	:
	m_startPoint(startPoint),
	m_endPoint(endPoint),
	m_controlPoints(controlPoints)
{

}

template<uint32_t Dim, uint32_t CoefficientCount>
inline void AInterpolater<Dim, CoefficientCount>::UpdateCoefficient()
{
	m_xProfiles.clear();

	m_xProfiles.emplace_back(m_startPoint.x);
	for (const SControlPoint<Dim>& controlPoint : m_controlPoints)
	{
		m_xProfiles.emplace_back(controlPoint.x);
	}
	m_xProfiles.emplace_back(m_endPoint.x);
}

template<uint32_t Dim, uint32_t CoefficientCount>
inline std::vector<SControlPoint<Dim>> AInterpolater<Dim, CoefficientCount>::GetControlPoints()
{
	std::vector<SControlPoint<Dim>> result;
	result.reserve(2 + m_controlPoints.size());
	result.emplace_back(m_startPoint);
	result.insert(result.end(), m_controlPoints.begin(), m_controlPoints.end());
	result.emplace_back(m_endPoint);
	return result;
}

template<uint32_t Dim, uint32_t CoefficientCount>
inline size_t AInterpolater<Dim, CoefficientCount>::GetCoefficientIndex(float x) noexcept
{
	const size_t xProfileStepCounts = m_xProfiles.size() - 1;
		
	for (size_t idx = 0; idx < xProfileStepCounts; ++idx)
	{
		if (m_xProfiles[idx] <= x && x < m_xProfiles[idx + 1])
		{
			return idx;
		}
	}
	return xProfileStepCounts - 1;
}
