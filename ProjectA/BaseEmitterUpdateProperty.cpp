#include "BaseEmitterUpdateProperty.h"
#include <exception>


using namespace std;
using namespace DirectX;
using namespace ImGui;

BaseEmitterUpdateProperty::BaseEmitterUpdateProperty(float& emitterCurrentTime)
	: m_emitterCurrentTime(emitterCurrentTime),
	m_isNotDisposed(true)
{
}

float BaseEmitterUpdateProperty::GetSpawnRate() const
{
	return m_spawnRateInterpolater->GetInterpolated(m_emitterCurrentTime);
}

void BaseEmitterUpdateProperty::SetSpawnControlPoints(const std::vector<SControlPoint>& spawnControlPoints) noexcept
{
	m_spawnControlPoints = spawnControlPoints;
	m_spawnRateInterpolater->GetCoefficients(spawnControlPoints);
}

void BaseEmitterUpdateProperty::SetSpawnRateInterpolationMethod(EInterpolationMethod spawnRateInterpolationMethod)
{
	m_spawnRateInterpolater = InterpolaterHelper::GetInterpolater(spawnRateInterpolationMethod);
	m_spawnRateInterpolationMethod = spawnRateInterpolationMethod;
}

void BaseEmitterUpdateProperty::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{

}

void BaseEmitterUpdateProperty::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	if (m_loopCount > 0)
	{
		m_emitterCurrentTime += dt;

		if (m_loopTime < m_emitterCurrentTime)
		{
			if (m_loopCount == LoopInfinity)
			{

			}
			else
			{
				m_emitterCurrentTime = max(m_emitterCurrentTime - m_loopTime, 0.f);
				m_loopCount -= 1;
			}
		}
	}
	else
	{
		if (m_isNotDisposed)
		{
			m_emitterDisposeHandler(this);
			m_isNotDisposed = false;
		}
	}
}

void BaseEmitterUpdateProperty::DrawPropertyUI()
{

}
//
//std::unique_ptr<BaseEmitterUpdateProperty> BaseEmitterUpdateProperty::DrawPropertyCreator(bool& isApplied, float& emitterCurrentTimeRef)
//{
//	static int loopCount = LoopInfinity;
//	static bool isLoopInfinity = true;
//	static float loopTime = 1.f;
//	static vector<SControlPoint> spawnControlPoints;
//	static EInterpolationMethod spawnInterpolateMethod = EInterpolationMethod::None;
//	static unique_ptr<AInterpolater> interpolater = InterpolaterHelper::GetInterpolater(EInterpolationMethod::None);
//	static bool isChanged = true;
//
//	bool makeProperty = false;
//
//	if (!ImGui::CollapsingHeader("이미터 업데이트 프로퍼티"))
//		return nullptr;
//
//	BeginDisabled(isApplied);
//	{
//		BeginDisabled(isLoopInfinity);
//		{
//			isChanged |= DragInt("루프 횟수", &loopCount, 1.f, 0, LoopInfinity - 1, isLoopInfinity ? "무한" : "%d");
//			EndDisabled();
//		}
//		SameLine();
//		if (Checkbox("무한 루프 설정", &isLoopInfinity))
//		{
//			loopCount = isLoopInfinity ? LoopInfinity : 1;
//		}
//
//		isChanged |= DragFloat("루프 당 시간", &loopTime, 0.1f, 0.f, 100.f, "%.1f");
//
//		EInterpolationMethod lastInterpolateKind = spawnInterpolateMethod;
//		InterpolationSelector::SelectEnums("생성 프로파일 보간 방법", InterpolationSelector::GInterpolationMethodStringMap, spawnInterpolateMethod);
//		if (lastInterpolateKind != spawnInterpolateMethod)
//		{		
//			interpolater = InterpolaterHelper::GetInterpolater(spawnInterpolateMethod);
//			size_t controlPointsCount = spawnControlPoints.size();
//
//			switch (spawnInterpolateMethod)
//			{
//			case EInterpolationMethod::None:
//				break;
//			case EInterpolationMethod::Linear:
//			case EInterpolationMethod::CubicSpline:
//			{
//				if (interpolater->IsInterpolatable(controlPointsCount))
//				{
//					interpolater->GetCoefficients(spawnControlPoints);
//				}
//				else
//				{
//					spawnInterpolateMethod = EInterpolationMethod::None;
//					interpolater = nullptr;
//					ImGui::OpenPopup("InterpolaterMessageAlarm");
//				}
//				break;
//			}
//			default:
//				break;
//			}
//			isChanged = true;
//		}
//
//		bool messageAlram = true;
//		if (ImGui::BeginPopupModal("InterpolaterMessageAlarm", &messageAlram))
//		{
//			Text("선형 보간의 경우 2개 이상의 컨트롤 포인트가, Cubic Spline 보간의 경우 3개 이상의 컨트롤 포인트가 필요합니다.");
//			if (ImGui::Button("닫기"))
//			{
//				ImGui::CloseCurrentPopup();
//			}
//			ImGui::EndPopup();
//		}
//
//		if (ImGui::Button("Spawn Rate 추가"))
//			ImGui::OpenPopup("AddSpawnRateControl");
//
//		bool unused_open = true;
//		if (ImGui::BeginPopupModal("AddSpawnRateControl", &unused_open))
//		{
//			static SControlPoint controlPoint;
//			DragFloat("컨트롤 포인트(x)", &controlPoint.x, 0.01f, spawnControlPoints.size() > 0 ? spawnControlPoints.back().x : 0.f, loopTime, "%.2f");
//			DragFloat("컨트롤 포인트(y)", &controlPoint.y, 1.f, 0.f, 1000.f, "%.f");
//			if (ImGui::Button("적용"))
//			{
//				spawnControlPoints.emplace_back(controlPoint);
//				if (interpolater) interpolater->GetCoefficients(spawnControlPoints);
//				controlPoint.x = min(loopTime, controlPoint.x + 1.f);
//				ImGui::CloseCurrentPopup();
//			}
//			SameLine();
//			if (ImGui::Button("취소"))
//			{
//				ImGui::CloseCurrentPopup();
//			}
//			ImGui::EndPopup();
//		}
//
//		InterpolationSelector::GridViewControlPoint1s("Spawn Rate", spawnControlPoints);
//		InterpolationSelector::ViewInterpolatedPoints(interpolater.get(), "Spawn Rate", "Spawn Control Points", "Spawn Interpolated", spawnControlPoints);
//		
//		EndDisabled();
//	}
//
//	BeginDisabled(!isChanged);
//	{
//		if (Button("이미터 업데이트 프로퍼티 설정"))
//		{
//			isChanged = false;
//			isApplied = true;
//			makeProperty = true;
//		}
//		else
//		{
//
//		}
//		EndDisabled();
//	}
//	SameLine();
//	BeginDisabled(isChanged);
//	{
//		if (Button("이미터 업데이트 프로퍼티 재설정"))
//		{
//			isChanged = true;
//			isApplied = false;
//		}
//		else
//		{
//
//		}
//		EndDisabled();
//	}
//
//	if (makeProperty)
//	{
//		return make_unique<BaseEmitterUpdateProperty>(
//			loopCount, loopTime, spawnControlPoints, spawnInterpolateMethod
//		);
//	}
//	return nullptr;
//
//}
