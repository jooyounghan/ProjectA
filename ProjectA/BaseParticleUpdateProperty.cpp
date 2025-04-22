#include "BaseParticleUpdateProperty.h"
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

void SetNForceCount(UINT& nForceCount, ENForceKind forceKind, UINT newValue)
{
	newValue = (newValue >= MaxNForceCount) ? (MaxNForceCount - 1) : newValue;
	
	constexpr UINT radixCount = GetRadixCount(MaxNForceCount);
	const UINT shift = GetRadixCount(MaxNForceCount) * static_cast<UINT>(forceKind);
	
	const UINT mask = ((1u << radixCount) - 1u) << shift;
	nForceCount &= ~mask;
	nForceCount |= (newValue << shift) & mask;
}

void IncrementNForceCount(UINT& nForceCount, ENForceKind forceKind)
{
	UINT count = GetNForceCount(nForceCount, forceKind);
	SetNForceCount(nForceCount, forceKind, count + 1);
}

void DecrementNForceCount(UINT& nForceCount, ENForceKind forceKind)
{
	UINT count = GetNForceCount(nForceCount, forceKind);
	return SetNForceCount(nForceCount, forceKind, count - 1);
}


BaseParticleUpdateProperty::BaseParticleUpdateProperty(
	bool& isEmitterForceChanged,
	SEmitterForceProperty& emitterForceProperty
)
	: m_isEmitterForceChanged(isEmitterForceChanged),
	m_emitterForceProperty(emitterForceProperty)
{
}

void BaseParticleUpdateProperty::SetFlag(EForceFlag forceFlag, bool isOn)
{
	isOn ? m_emitterForceProperty.forceFlag |= GetForceFlagOffset(forceFlag) : m_emitterForceProperty.forceFlag &= ~GetForceFlagOffset(forceFlag);
	m_isEmitterForceChanged = true;
}

void BaseParticleUpdateProperty::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{

}

void BaseParticleUpdateProperty::Update(ID3D11DeviceContext* deviceContext, float dt)
{

}

void BaseParticleUpdateProperty::DrawPropertyUI()
{
	if (!CollapsingHeader("파티클 업데이트 프로퍼티"))
		return;
	

	HandleSingleForce("중력", EForceFlag::Gravity, [&]()
		{
			DragFloat3("중력 벡터", &m_emitterForceProperty.gravityForce.x, 0.1f, -1000.f, 1000.f, "%.1f");
		}
	);

	HandleSingleForce("항력", EForceFlag::Drag, [&]()
		{
			DragFloat("항력계수", &m_emitterForceProperty.dragCoefficient, 0.01f, 0.f, 1.f, "%.2f");
		}
	);

	HandleSingleForce("Curl-Noise", EForceFlag::CurNoise, [&]()
		{
			DragFloat("Curl-Noise Octave", &m_emitterForceProperty.curlNoiseOctave, 0.01f, 0.f, 10.f, "%.2f");
			DragFloat("Curl-Noise Coefficient", &m_emitterForceProperty.curlNoiseCoefficient, 0.01f, 0.f, 10.f, "%.2f");
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
			SVortexForce& vortexForce = m_emitterForceProperty.nVortexForce[currentIndex];
			DragFloat3("Vortex 원점", &vortexForce.vortexOrigin.x, 0.1f, -1000.f, 1000.f, "%.1f");
			DragFloat3("Vortex 축", &vortexForce.vortexAxis.x, 0.01f, -1.f, 1.f, "%.2f");
			DragFloat("작용 반지름", &vortexForce.vortexRadius, 0.1f, 0.f, 1000.f, "%.1f");
			DragFloat("지평선 반지름", &vortexForce.vortexDeathHorizonRadius, 0.1f, 0.f, vortexForce.vortexRadius, "%.1f");
			DragFloat("Vortex 계수", &vortexForce.vortextCoefficient, 0.01f, 0.f, 10.f, "%.2f");
			DragFloat("중심 끌림 강도", &vortexForce.vortexTightness, 0.01f, 0.f, 10.f, "%.2f");
		}
	);

	HandleNForce("선 인력", EForceFlag::LineInteraction, ENForceKind::LineInteraction,
		[&](UINT addIndex) {
			SLineInteractionForce& lineInteractionForce = m_emitterForceProperty.nLineInteractionForce[addIndex];
			AutoZeroMemory(lineInteractionForce);
		},
		[&](UINT deleteIndex) {
			memmove(&m_emitterForceProperty.nLineInteractionForce[deleteIndex],
				&m_emitterForceProperty.nLineInteractionForce[deleteIndex + 1],
				sizeof(SLineInteractionForce) * (MaxNForceCount - (deleteIndex + 1)));
		},
		[&](UINT currentIndex) {
			SLineInteractionForce& lineInteractionForce = m_emitterForceProperty.nLineInteractionForce[currentIndex];
			DragFloat3("선 정점", &lineInteractionForce.lineInteractionOrigin.x, 0.1f, -1000.f, 1000.f, "%.1f");
			DragFloat3("선 방향", &lineInteractionForce.lineInteractionAxis.x, 0.01f, -1.f, 1.f, "%.2f");
			DragFloat("영향 길이", &lineInteractionForce.interactionDistance, 0.1f, 0.f, 1000.f, "%.1f");
			DragFloat("인력 계수", &lineInteractionForce.interactionCoefficient, 0.01f, 0.f, 10.f, "%.2f");
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
			SPointInteractionForce& pointInteractionForce = m_emitterForceProperty.nPointInteractionForce[currentIndex];
			DragFloat3("정점", &pointInteractionForce.pointInteractionCenter.x, 0.1f, -1000.f, 1000.f, "%.1f");
			DragFloat("영향 반지름", &pointInteractionForce.interactionRadius, 0.1f, 0.f, 1000.f, "%.1f");
			DragFloat("인력 계수", &pointInteractionForce.interactionCoefficient, 0.01f, 0.f, 10.f, "%.2f");
		}
	);
}

void BaseParticleUpdateProperty::HandleSingleForce(
	const string& forceName,
	EForceFlag force,
	const function<void()>& handler
)
{
	bool isForceOn = IsForceOn(force);
	SeparatorText(forceName.c_str());
	BeginDisabled(!isForceOn);
	{
		handler();
	}
	EndDisabled();
	SameLine();
	if (Checkbox(format("##{}CheckBox", forceName).c_str(), &isForceOn))
	{
		isForceOn ? SetFlag(force, true) : SetFlag(force, false);
	}

}

void BaseParticleUpdateProperty::HandleNForce(
	const string& forceName,
	EForceFlag forceFlag, 
	ENForceKind nForceKind,
	const function<void(UINT)>& addButtonHandler,
	const function<void(UINT)>& deleteButtonHandler,
	const function<void(UINT)>& handler
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
				handler(forceIdx);
			}
			PopID();
		}

		for (auto& removedVortexIdx : removedForcesIdx)
		{
			DecrementNForceCount(m_emitterForceProperty.nForceCount, ENForceKind::Vortex);
			if (nForceCount == 1)
			{
				SetFlag(EForceFlag::Vortex, false);
				break;
			}
			else if (nForceCount == MaxNForceCount)
			{
				m_isEmitterForceChanged = true;
			}
			else
			{
				deleteButtonHandler(removedVortexIdx);
				m_isEmitterForceChanged = true;
			}
		}

	}
	EndDisabled();
}
