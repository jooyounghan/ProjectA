#pragma once
#include "InterpolaterStructure.h"
#include "GPUInterpolater.h"

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

template<uint32_t Dim, bool GPUInterpolateOn>
class IInterpolater
{
public:
	virtual ~IInterpolater() = default;

public:
	virtual uint32_t GetInterpolaterID() = 0;
	virtual uint32_t GetCoefficientCount() = 0;
	virtual void UpdateCoefficient() = 0;
	virtual std::array<float, Dim> GetInterpolated(float x) noexcept = 0;
};

template<uint32_t Dim, uint32_t CoefficientCount, bool GPUInterpolateOn>
class AInterpolater : public IInterpolater<Dim, GPUInterpolateOn>
{
public:
	AInterpolater(
		const SControlPoint<Dim>& startPoint,
		const SControlPoint<Dim>& endPoint,
		const std::vector<SControlPoint<Dim>>& controlPoints
	);
	~AInterpolater() override;

protected:
	const SControlPoint<Dim>& m_startPoint;
	const SControlPoint<Dim>& m_endPoint;
	const std::vector<SControlPoint<Dim>>& m_controlPoints;

protected:
	std::vector<float> m_xProfiles;
	using CoeffType = std::array<float, CoefficientCount * Dim>;
	std::vector<CoeffType> m_coefficients;

protected:
	UINT m_interpolaterPropertyID;
	SInterpolaterProperty<Dim, CoefficientCount>* m_interpolaterPropertyCached;

public:
	void UpdateInterpolaterProperty();
	
public:
	virtual UINT GetInterpolaterFlag() = 0;

public:
	virtual UINT GetInterpolaterID() override { return m_interpolaterPropertyID; }
	virtual uint32_t GetCoefficientCount() override { return CoefficientCount; };
	virtual void UpdateCoefficient() override;

protected:
	virtual std::vector<SControlPoint<Dim>> GetControlPoints();

protected:
	size_t GetCoefficientIndex(float x) noexcept;
};

template<uint32_t Dim, uint32_t CoefficientCount, bool GPUInterpolateOn>
void AInterpolater<Dim, CoefficientCount, GPUInterpolateOn>::UpdateInterpolaterProperty()
{
	if (GPUInterpolateOn)
	{
		m_interpolaterPropertyCached->UpdateInterpolaterProperty(
			GetInterpolaterFlag(),
			m_xProfiles,
			m_coefficients
		);
		GPUInterpolater<Dim, CoefficientCount>::GChangedInterpolaterIDs.emplace_back(m_interpolaterPropertyID);
	}
}

template<uint32_t Dim, uint32_t CoefficientCount, bool GPUInterpolateOn>
inline AInterpolater<Dim, CoefficientCount, GPUInterpolateOn>::AInterpolater(
	const SControlPoint<Dim>& startPoint, 
	const SControlPoint<Dim>& endPoint, 
	const std::vector<SControlPoint<Dim>>& controlPoints
)
	:
	m_startPoint(startPoint),
	m_endPoint(endPoint),
	m_controlPoints(controlPoints)
{
	if (GPUInterpolateOn)
	{
		m_interpolaterPropertyID = GPUInterpolater<Dim, CoefficientCount>::IssueAvailableInterpolaterID();
		m_interpolaterPropertyCached = GPUInterpolater<Dim, CoefficientCount>::GetInterpolaterProperty(m_interpolaterPropertyID);
	}
}

template<uint32_t Dim, uint32_t CoefficientCount, bool GPUInterpolateOn>
AInterpolater<Dim, CoefficientCount, GPUInterpolateOn>::~AInterpolater()
{
	if (GPUInterpolateOn)
	{
		GPUInterpolater<Dim, CoefficientCount>::ReclaimInterpolaterID(m_interpolaterPropertyID);
		GPUInterpolater<Dim, CoefficientCount>::GChangedInterpolaterIDs.emplace_back(m_interpolaterPropertyID);
		ZeroMemory(m_interpolaterPropertyCached, sizeof(SInterpolaterProperty<Dim, CoefficientCount>));
		m_interpolaterPropertyCached = nullptr;
	}
}

template<uint32_t Dim, uint32_t CoefficientCount, bool GPUInterpolateOn>
inline void AInterpolater<Dim, CoefficientCount, GPUInterpolateOn>::UpdateCoefficient()
{
	m_xProfiles.clear();

	m_xProfiles.emplace_back(m_startPoint.x);
	for (const SControlPoint<Dim>& controlPoint : m_controlPoints)
	{
		m_xProfiles.emplace_back(controlPoint.x);
	}
	m_xProfiles.emplace_back(m_endPoint.x);
}

template<uint32_t Dim, uint32_t CoefficientCount, bool GPUInterpolateOn>
inline std::vector<SControlPoint<Dim>> AInterpolater<Dim, CoefficientCount, GPUInterpolateOn>::GetControlPoints()
{
	std::vector<SControlPoint<Dim>> result;
	result.reserve(2 + m_controlPoints.size());
	result.emplace_back(m_startPoint);
	result.insert(result.end(), m_controlPoints.begin(), m_controlPoints.end());
	result.emplace_back(m_endPoint);
	return result;
}

template<uint32_t Dim, uint32_t CoefficientCount, bool GPUInterpolateOn>
inline size_t AInterpolater<Dim, CoefficientCount, GPUInterpolateOn>::GetCoefficientIndex(float x) noexcept
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
