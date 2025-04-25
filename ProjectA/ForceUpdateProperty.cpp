#include "ForceUpdateProperty.h"
#include "MacroUtilities.h"
#include "imgui.h"

#include <exception>
#include <format>
#include <vector>

using namespace std;
using namespace DirectX;
using namespace ImGui;

constexpr UINT GetForceFlagOffset(EForceFlag forceFlag)
{
	return 1 << static_cast<UINT>(forceFlag);
}

constexpr UINT GetNForceCount(UINT nForceCount, ENForceKind forceKind)
{
	constexpr UINT radixCount = GetRadixCount(MaxNForceCount);
	const UINT shift = radixCount * static_cast<UINT>(forceKind);
	return (nForceCount >> shift) & ((1 << radixCount) - 1);
}

constexpr void SetNForceCount(UINT& nForceCount, ENForceKind forceKind, UINT newValue)
{
	newValue = (newValue >= MaxNForceCount) ? (MaxNForceCount - 1) : newValue;
	
	constexpr UINT radixCount = GetRadixCount(MaxNForceCount);
	const UINT shift = GetRadixCount(MaxNForceCount) * static_cast<UINT>(forceKind);
	
	const UINT mask = ((1u << radixCount) - 1u) << shift;
	nForceCount &= ~mask;
	nForceCount |= (newValue << shift) & mask;
}

constexpr void IncrementNForceCount(UINT& nForceCount, ENForceKind forceKind)
{
	UINT count = GetNForceCount(nForceCount, forceKind);
	SetNForceCount(nForceCount, forceKind, count + 1);
}

constexpr void DecrementNForceCount(UINT& nForceCount, ENForceKind forceKind)
{
	UINT count = GetNForceCount(nForceCount, forceKind);
	return SetNForceCount(nForceCount, forceKind, count - 1);
}


ForceUpdateProperty::ForceUpdateProperty(
	const std::function<void(const SEmitterForceProperty&)>& emitterForceUpdatedHandler
)
	: m_isEmitterForcePropertyChanged(false),
	m_onEmitterForceUpdated(emitterForceUpdatedHandler)
{
	AutoZeroMemory(m_emitterForceProperty);
}


void ForceUpdateProperty::SetFlag(EForceFlag forceFlag, bool isOn)
{
	isOn ? m_emitterForceProperty.forceFlag |= GetForceFlagOffset(forceFlag) : m_emitterForceProperty.forceFlag &= ~GetForceFlagOffset(forceFlag);
	m_isEmitterForcePropertyChanged = true;
}

void ForceUpdateProperty::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{

}

void ForceUpdateProperty::Update(ID3D11DeviceContext* deviceContext, float dt)
{
	if (m_isEmitterForcePropertyChanged)
	{
		m_onEmitterForceUpdated(m_emitterForceProperty);
		m_isEmitterForcePropertyChanged = false;
	}
}

void ForceUpdateProperty::DrawPropertyUI()
{
	if (!CollapsingHeader("파티클 업데이트 프로퍼티"))
		return;
	

	HandleSingleForce("중력", EForceFlag::Gravity, [&]()
		{
			return DragFloat3("중력 벡터", &m_emitterForceProperty.gravityForce.x, 0.1f, -1000.f, 1000.f, "%.1f");
		}
	);

	HandleSingleForce("항력", EForceFlag::Drag, [&]()
		{
			return DragFloat("항력계수", &m_emitterForceProperty.dragCoefficient, 0.01f, 0.f, 1.f, "%.2f");
		}
	);

	HandleSingleForce("Curl-Noise", EForceFlag::CurNoise, [&]()
		{
			bool isChanged = false;
			isChanged |= DragFloat("Curl-Noise Octave", &m_emitterForceProperty.curlNoiseOctave, 0.01f, 0.f, 10.f, "%.2f");
			isChanged |= DragFloat("Curl-Noise Coefficient", &m_emitterForceProperty.curlNoiseCoefficient, 0.01f, 0.f, 10.f, "%.2f");
			return isChanged;
		}
	);

	HandleNForce("Vortex", EForceFlag::Vortex, ENForceKind::Vortex,
		[&](UINT addIndex) { 	
			SVortexForce& vortexForce = m_emitterForceProperty.nVortexForce[addIndex];
			AutoZeroMemory(vortexForce);
		},
		[&](UINT deleteIndex) {
			memmove(&m_emitterForceProperty.nVortexForce[deleteIndex],
			&m_emitterForceProperty.nVortexForce[deleteIndex + 1],
			sizeof(SVortexForce) * (MaxNForceCount - (deleteIndex + 1)));
		},
		[&](UINT currentIndex) {
			bool isChanged = false;
			SVortexForce& vortexForce = m_emitterForceProperty.nVortexForce[currentIndex];
			isChanged |= DragFloat3("Vortex 원점", &vortexForce.vortexOrigin.x, 0.1f, -1000.f, 1000.f, "%.1f");
			isChanged |= DragFloat3("Vortex 축", &vortexForce.vortexAxis.x, 0.01f, -1.f, 1.f, "%.2f");
			isChanged |= DragFloat("작용 반지름", &vortexForce.vortexRadius, 0.1f, 0.f, 1000.f, "%.1f");
			isChanged |= DragFloat("지평선 반지름", &vortexForce.vortexDeathHorizonRadius, 0.1f, 0.f, vortexForce.vortexRadius, "%.1f");
			isChanged |= DragFloat("Vortex 계수", &vortexForce.vortextCoefficient, 0.01f, 0.f, 10.f, "%.2f");
			isChanged |= DragFloat("중심 끌림 강도", &vortexForce.vortexTightness, 0.01f, 0.f, 10.f, "%.2f");
			return isChanged;
		}
	);

	HandleNForce("점 인력", EForceFlag::PointInteraction, ENForceKind::PointInteraction,
		[&](UINT addIndex) {
			SPointInteractionForce& pointInteractionForce = m_emitterForceProperty.nPointInteractionForce[addIndex];
			AutoZeroMemory(pointInteractionForce);
		},
		[&](UINT deleteIndex) {
			memmove(&m_emitterForceProperty.nPointInteractionForce[deleteIndex],
				&m_emitterForceProperty.nPointInteractionForce[deleteIndex + 1],
				sizeof(SPointInteractionForce) * (MaxNForceCount - (deleteIndex + 1)));
		},
		[&](UINT currentIndex) {
			bool isChanged = false;
			SPointInteractionForce& pointInteractionForce = m_emitterForceProperty.nPointInteractionForce[currentIndex];
			isChanged |= DragFloat3("정점", &pointInteractionForce.pointInteractionCenter.x, 0.1f, -1000.f, 1000.f, "%.1f");
			isChanged |= DragFloat("영향 반지름", &pointInteractionForce.interactionRadius, 0.1f, 0.f, 1000.f, "%.1f");
			isChanged |= DragFloat("인력 계수", &pointInteractionForce.interactionCoefficient, 0.01f, 0.f, 10.f, "%.2f");
			return isChanged;
		}
	);
}

void ForceUpdateProperty::HandleSingleForce(
	const string& forceName,
	EForceFlag force,
	const function<bool()>& handler
)
{
	bool isForceOn = IsForceOn(force);
	SeparatorText(forceName.c_str());
	BeginDisabled(!isForceOn);
	{
		if (handler()) m_isEmitterForcePropertyChanged = true;
	}
	EndDisabled();
	SameLine();
	if (Checkbox(format("##{}CheckBox", forceName).c_str(), &isForceOn))
	{
		isForceOn ? SetFlag(force, true) : SetFlag(force, false);
	}

}

void ForceUpdateProperty::HandleNForce(
	const string& forceName,
	EForceFlag forceFlag, 
	ENForceKind nForceKind,
	const function<void(UINT)>& addButtonHandler,
	const function<void(UINT)>& deleteButtonHandler,
	const function<bool(UINT)>& handler
)
{
	UINT nForceCount = GetNForceCount(m_emitterForceProperty.nForceCount, nForceKind);
	bool isnForceSet = IsForceOn(forceFlag);

	SeparatorText(forceName.c_str());
	BeginDisabled(nForceCount >= MaxNForceCount);
	{
		if (Button(format("{} 추가", forceName).c_str()))
		{
			IncrementNForceCount(m_emitterForceProperty.nForceCount, nForceKind);
			addButtonHandler(nForceCount);
			SetFlag(forceFlag, true);
		}
	}
	EndDisabled();

	BeginDisabled(!isnForceSet);
	{
		vector<UINT> removedForcesIdx;
		for (UINT forceIdx = 0; forceIdx < nForceCount; ++forceIdx)
		{
			PushID(format("{}Property{}", forceName, forceIdx).c_str());
			if (Button("삭제"))
			{
				removedForcesIdx.emplace_back(forceIdx);
			}
			SameLine();
			if (CollapsingHeader(format("{}{} 프로퍼티", forceName, (forceIdx + 1)).c_str()))
			{
				if (handler(forceIdx)) m_isEmitterForcePropertyChanged = true;
			}
			PopID();
		}

		for (auto& removedVortexIdx : removedForcesIdx)
		{
			DecrementNForceCount(m_emitterForceProperty.nForceCount, nForceKind);
			if (nForceCount == 1)
			{
				SetFlag(forceFlag, false);
				break;
			}
			else if (nForceCount == MaxNForceCount)
			{
				m_isEmitterForcePropertyChanged = true;
			}
			else
			{
				deleteButtonHandler(removedVortexIdx);
				m_isEmitterForcePropertyChanged = true;
			}
		}

	}
	EndDisabled();
}

void ForceUpdateProperty::Serialize(std::ofstream& ofs)
{
	SerializeHelper::SerializeElement<SEmitterForceProperty>(ofs, m_emitterForceProperty);

}

void ForceUpdateProperty::Deserialize(std::ifstream& ifs)
{
	m_emitterForceProperty = SerializeHelper::DeserializeElement<SEmitterForceProperty>(ifs);
	m_isEmitterForcePropertyChanged = true;
}
