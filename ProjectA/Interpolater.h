#pragma once
#include <array>
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
struct SControlPoint
{
	float x = 0.f;
	std::array<float, Dim> y;
};

template<uint32_t Dim>
class IInterpolater
{
public:
	virtual std::array<float, Dim> GetInterpolated(float x) noexcept = 0;
};

template<uint32_t Dim, size_t Multiplier = 2>
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
	std::vector<float> xProfiles;
	using CoeffType = std::array<float, Multiplier * Dim>;
	std::vector<CoeffType> coefficients;

protected:
	virtual std::vector<SControlPoint<Dim>> GetControlPoints(
		const SControlPoint<Dim>& startPoint,
		const SControlPoint<Dim>& endPoint,
		const std::vector<SControlPoint<Dim>>& controlPoints
	);

protected:
	size_t GetCoefficientIndex(float x) noexcept;
};

template<uint32_t Dim, size_t Multiplier>
inline AInterpolater<Dim, Multiplier>::AInterpolater(
	const SControlPoint<Dim>& startPoint, 
	const SControlPoint<Dim>& endPoint, 
	const std::vector<SControlPoint<Dim>>& controlPoints
)
{
	xProfiles.emplace_back(startPoint.x);
	for (const SControlPoint<Dim>& controlPoint : controlPoints)
	{
		xProfiles.emplace_back(controlPoint.x);
	}
	xProfiles.emplace_back(endPoint.x);
}

template<uint32_t Dim, size_t Multiplier>
inline std::vector<SControlPoint<Dim>> AInterpolater<Dim, Multiplier>::GetControlPoints(
	const SControlPoint<Dim>& startPoint, 
	const SControlPoint<Dim>& endPoint, 
	const std::vector<SControlPoint<Dim>>& controlPoints
)
{
	std::vector<SControlPoint<Dim>> result;
	result.reserve(2 + controlPoints.size());
	result.emplace_back(startPoint);
	result.insert(result.end(), controlPoints.begin(), controlPoints.end());
	result.emplace_back(endPoint);
	return result;
}

template<uint32_t Dim, size_t Multiplier>
inline size_t AInterpolater<Dim, Multiplier>::GetCoefficientIndex(float x) noexcept
{
	const size_t xProfileStepCounts = xProfiles.size() - 1;
		
	for (size_t idx = 0; idx < xProfileStepCounts; ++idx)
	{
		if (xProfiles[idx] <= x && x < xProfiles[idx + 1])
		{
			return idx;
		}
	}
	return xProfileStepCounts - 1;
}
