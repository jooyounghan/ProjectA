#include "BaseParticleUpdateProperty.h"
#include "MacroUtilities.h"
#include "imgui.h"

#include <exception>
#include <format>
#include <string>

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

#define GetFlag(flag)												\
(m_emitterForceProperty.forceFlag >> static_cast<UINT>(flag)) & 0b1	\

#define SetOnFlag(flag)												\
m_emitterForceProperty.forceFlag |= GetForceFlagOffset(flag);		\
m_isEmitterForceChanged = true;										\

#define SetOffFlag(flag)												\
m_emitterForceProperty.forceFlag &= !GetForceFlagOffset(flag);		\
m_isEmitterForceChanged = true;										\

void BaseParticleUpdateProperty::AddLineInteractionForce(
	const XMFLOAT3 lineInteractionOrigin, 
	const XMFLOAT3 lineInteractionAxis,
	float interactionDistance,
	float interactionCoefficient
)
{
	UINT lineInteractionCount = GetNForceCount(m_emitterForceProperty.nForceCount, ENForceKind::LineInteraction);
	if (lineInteractionCount == MaxNForceCount)
	{
		throw exception("Line Interaction Count Exceed Maximum Value");
	}
	else
	{
		SetOnFlag(EForceFlag::LineInteraction);
		IncrementNForceCount(m_emitterForceProperty.nForceCount, ENForceKind::LineInteraction);
		SLineInteractionForce& lineInteractionForce = m_emitterForceProperty.nLineInteractionForce[lineInteractionCount];
		lineInteractionForce.lineInteractionOrigin = lineInteractionOrigin;
		lineInteractionForce.lineInteractionAxis = lineInteractionAxis;
		lineInteractionForce.interactionDistance = interactionDistance;
		lineInteractionForce.interactionCoefficient = interactionCoefficient;
	}
}

void BaseParticleUpdateProperty::RemoveLineInteractionForce(UINT lineInteractionForceIndex)
{
	UINT lineInteractionCount = GetNForceCount(m_emitterForceProperty.nForceCount, ENForceKind::LineInteraction);
	if (lineInteractionCount == 0)
	{
		throw exception("No Line Interaction To Remove");
	}
	else
	{
		DecrementNForceCount(m_emitterForceProperty.nForceCount, ENForceKind::LineInteraction);
		if (lineInteractionCount == 1)
		{
			SetOffFlag(EForceFlag::LineInteraction);
		}
		else if (lineInteractionCount == MaxNForceCount)
		{
			m_isEmitterForceChanged = true;
		}
		else
		{
			std::memmove(&m_emitterForceProperty.nLineInteractionForce[lineInteractionForceIndex],
				&m_emitterForceProperty.nLineInteractionForce[lineInteractionForceIndex + 1],
				sizeof(SLineInteractionForce) * (MaxNForceCount - (lineInteractionForceIndex + 1))
			);
			m_isEmitterForceChanged = true;
		}
	}
}

void BaseParticleUpdateProperty::AddPointInteractionForce(
	XMFLOAT3 pointInteractionCenter,
	float interactionRadius,
	float interactionCoefficient
)
{
	UINT pointInteractionCount = GetNForceCount(m_emitterForceProperty.nForceCount, ENForceKind::PointInteraction);
	if (pointInteractionCount == MaxNForceCount)
	{
		throw exception("Point Interaction Count Exceed Maximum Value");
	}
	else
	{
		SetOnFlag(EForceFlag::PointInteraction);
		IncrementNForceCount(m_emitterForceProperty.nForceCount, ENForceKind::PointInteraction);
		SPointInteractionForce& pointInteractionForce = m_emitterForceProperty.nPointInteractionForce[pointInteractionCount];
		pointInteractionForce.pointInteractionCenter = pointInteractionCenter;
		pointInteractionForce.interactionRadius = interactionRadius;
		pointInteractionForce.interactionCoefficient = interactionCoefficient;
	}
}

void BaseParticleUpdateProperty::RemovePointInteractionForce(UINT pointInteractionForceIndex)
{
	UINT pointInteractionCount = GetNForceCount(m_emitterForceProperty.nForceCount, ENForceKind::PointInteraction);
	if (pointInteractionCount == 0)
	{
		throw exception("No Point Interaction To Remove");
	}
	else
	{
		DecrementNForceCount(m_emitterForceProperty.nForceCount, ENForceKind::PointInteraction);
		if (pointInteractionCount == 1)
		{
			SetOffFlag(EForceFlag::PointInteraction);
		}
		else if (pointInteractionCount == MaxNForceCount)
		{
			m_isEmitterForceChanged = true;
		}
		else
		{
			std::memmove(&m_emitterForceProperty.nPointInteractionForce[pointInteractionForceIndex],
				&m_emitterForceProperty.nPointInteractionForce[pointInteractionForceIndex + 1],
				sizeof(SPointInteractionForce) * (MaxNForceCount - (pointInteractionForceIndex + 1))
			);
			m_isEmitterForceChanged = true;
		}
	}
}

void BaseParticleUpdateProperty::Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext)
{

}

void BaseParticleUpdateProperty::Update(ID3D11DeviceContext* deviceContext, float dt)
{

}

void BaseParticleUpdateProperty::DrawPropertyUI()
{
	bool isGravitySet = GetFlag(EForceFlag::Gravity);
	bool isDragSet = GetFlag(EForceFlag::Drag);
	bool isCurNoiseSet = GetFlag(EForceFlag::CurNoise);

	if (!CollapsingHeader("파티클 업데이트 프로퍼티"))
		return;
	
	SeparatorText("중력");
	BeginDisabled(!isGravitySet);
	{
		DragFloat3("중력 벡터", &m_emitterForceProperty.gravityForce.x, 0.1f, -1000.f, 1000.f, "%.1f");
	}
	EndDisabled();
	SameLine();
	if (Checkbox("##GravityForceCheckBox", &isGravitySet))
	{
		if (isGravitySet)
		{
			SetOnFlag(EForceFlag::Gravity);
		}
		else
		{
			SetOffFlag(EForceFlag::Gravity);
		}
	}

	SeparatorText("항력");
	BeginDisabled(!isDragSet);
	{
		DragFloat("항력계수", &m_emitterForceProperty.dragCoefficient, 0.1f, 0.f, 1000.f, "%.1f");
	}
	EndDisabled();
	SameLine();
	if (Checkbox("##DragCoefficientCheckBox", &isDragSet))
	{
		if (isDragSet)
		{
			SetOnFlag(EForceFlag::Drag);
		}
		else
		{
			SetOffFlag(EForceFlag::Drag);
		}
	}

	SeparatorText("Curl-Noise");
	BeginDisabled(!isCurNoiseSet);
	{
		DragFloat("Curl-Noise Octave", &m_emitterForceProperty.curlNoiseOctave, 0.1f, 0.f, 1000.f, "%.1f");
		DragFloat("Curl-Noise Coefficient", &m_emitterForceProperty.curlNoiseCoefficient, 0.1f, 0.f, 1000.f, "%.1f");
	}
	EndDisabled();
	SameLine();
	if (Checkbox("##CurlNoiseCheckBox", &isCurNoiseSet))
	{
		if (isCurNoiseSet)
		{
			SetOnFlag(EForceFlag::CurNoise);
		}
		else
		{
			SetOffFlag(EForceFlag::CurNoise);
		}
	}

	bool isVortexSet = GetFlag(EForceFlag::Vortex);
	bool isLineInteractionSet = GetFlag(EForceFlag::LineInteraction);
	bool isPointInteractionSet = GetFlag(EForceFlag::PointInteraction);
	UINT vortexCount = GetNForceCount(m_emitterForceProperty.nForceCount, ENForceKind::Vortex);
	UINT lineInteractionCount = GetNForceCount(m_emitterForceProperty.nForceCount, ENForceKind::LineInteraction);
	UINT pointInteractionCount = GetNForceCount(m_emitterForceProperty.nForceCount, ENForceKind::PointInteraction);

	SeparatorText("Vortex");

	if (Button("Add Vortex"))
	{
		IncrementNForceCount(m_emitterForceProperty.nForceCount, ENForceKind::Vortex);
	}
	if (Button("Line Interaction Vortex"))
	{
		IncrementNForceCount(m_emitterForceProperty.nForceCount, ENForceKind::LineInteraction);
	}
	if (Button("Point Interaction Vortex"))
	{
		IncrementNForceCount(m_emitterForceProperty.nForceCount, ENForceKind::PointInteraction);
	}
	Text(to_string(GetNForceCount(m_emitterForceProperty.nForceCount, ENForceKind::Vortex)).c_str());
	Text(to_string(GetNForceCount(m_emitterForceProperty.nForceCount, ENForceKind::LineInteraction)).c_str());
	Text(to_string(GetNForceCount(m_emitterForceProperty.nForceCount, ENForceKind::PointInteraction)).c_str());

//	BeginDisabled(!isVortexSet);
//	{
//		for (UINT vortexIdx = 0; vortexIdx < vortexCount; ++vortexIdx)
//		{
//			PushID(format("VortextForceProperty{}", vortexIdx).c_str());
//			Button("-");
//			SameLine();
//			if (!CollapsingHeader(format("Vortext Force Property{}", (vortexIdx + 1)).c_str()))
//				continue;
//
//			SVortexForce& vortexForce = m_emitterForceProperty.nVortexForce[vortexIdx];
//			DragFloat3("Vortex 원점", &vortexForce.vortexOrigin.x, 0.1f, -1000.f, 1000.f, ".%1f");
//			DragFloat3("Vortex 축", &vortexForce.vortexAxis.x, 0.01f, -1.f, 1.f, ".%2f");
//			DragFloat("Radius", &vortexForce.vortexRadius, 0.1f, 0.f, 1000.f, ".%1f");
//			DragFloat("Vortex Coefficient", &vortexForce.vortextCoefficient, 0.01f, 0.f, 100.f, ".%2f");
//			DragFloat("Vortex Tightness", &vortexForce.vortexTightness, 0.01f, 0.f, 100.f, ".%2f");
//			
//			PopID();
//		}
//	}
//	EndDisabled();
//	SameLine();
//	BeginDisabled(vortexCount >= MaxNForceCount);
//	{
//		if (Button("Add Vortex"))
//		{
//			IncrementNForceCount(m_emitterForceProperty.nForceCount, ENForceKind::Vortex);
//			SVortexForce& vortexForce = m_emitterForceProperty.nVortexForce[vortexCount];
//			AutoZeroMemory(vortexForce);
//		}
//	}
//	EndDisabled();
//}

//
//
//void BaseParticleUpdateProperty::AddVortexForce(
//	const XMFLOAT3 vortexOrigin,
//	const XMFLOAT3 vortexAxis,
//	float vortexRadius,
//	float vortexTightness
//)
//{
//	UINT vortexCount = GetNForceCount(m_emitterForceProperty.nForceCount, ENForceKind::Vortex);
//	if (vortexCount == MaxNForceCount)
//	{
//		throw exception("Vortex Count Exceed Maximum Value");
//	}
//	else
//	{
//		SetOnFlag(EForceFlag::Vortex);

//		SVortexForce& vortexForce = m_emitterForceProperty.nVortexForce[vortexCount];
//		vortexForce.vortexOrigin = vortexOrigin;
//		vortexForce.vortexAxis = vortexAxis;
//		vortexForce.vortexRadius = vortexRadius;
//		vortexForce.vortexTightness = vortexTightness;
//	}
//}
//
//void BaseParticleUpdateProperty::RemoveVortexForce(UINT vortexForceIndex)
//{
//	UINT vortexCount = GetNForceCount(m_emitterForceProperty.nForceCount, ENForceKind::Vortex);
//	if (vortexCount == 0)
//	{
//		throw exception("No Vortex To Remove");
//	}
//	else
//	{
//		DecrementNForceCount(m_emitterForceProperty.nForceCount, ENForceKind::Vortex);
//		if (vortexCount == 1)
//		{
//			SetOffFlag(EForceFlag::Vortex);
//		}
//		else if (vortexCount == MaxNForceCount)
//		{
//			m_isEmitterForceChanged = true;
//		}
//		else
//		{
//			std::memmove(&m_emitterForceProperty.nVortexForce[vortexForceIndex],
//				&m_emitterForceProperty.nVortexForce[vortexForceIndex + 1],
//				sizeof(SVortexForce) * (MaxNForceCount - (vortexForceIndex + 1))
//			);
//			m_isEmitterForceChanged = true;
//		}
//	}
}