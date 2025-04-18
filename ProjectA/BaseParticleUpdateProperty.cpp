#include "BaseParticleUpdateProperty.h"
#include <exception>
#include <cstring>

#include "imgui.h"

using namespace std;
using namespace DirectX;
using namespace ImGui;

constexpr UINT GetForceFlagOffset(EForceFlag forceFlag)
{
	return 1 << static_cast<UINT>(forceFlag);
}

constexpr UINT GetNForceCount(UINT nForceCount, ENForceKind forceKind)
{
	return (nForceCount >> (ceil_log2(MaxNForceCount)) * static_cast<UINT>(forceKind)) & (MaxNForceCount - 1);
}

constexpr UINT SetNForceCount(UINT nForceCount, ENForceKind forceKind, UINT newValue)
{
	constexpr UINT bitsPerForce = ceil_log2(MaxNForceCount);
	const UINT shift = bitsPerForce * static_cast<UINT>(forceKind);
	const UINT mask = ((1u << bitsPerForce) - 1u) << shift;

	newValue = (newValue >= MaxNForceCount) ? (MaxNForceCount - 1) : newValue;

	return (nForceCount & ~mask) | ((newValue & ((1u << bitsPerForce) - 1u)) << shift);
}

constexpr UINT IncrementNForceCount(UINT nForceCount, ENForceKind forceKind)
{
	UINT count = GetNForceCount(nForceCount, forceKind);
	return SetNForceCount(nForceCount, forceKind, count + 1);
}

constexpr UINT DecrementNForceCount(UINT nForceCount, ENForceKind forceKind)
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

#define OnFlag(flag)												\
m_emitterForceProperty.forceFlag |= GetForceFlagOffset(flag);		\
m_isEmitterForceChanged = true;										\

#define OffFlag(flag)												\
m_emitterForceProperty.forceFlag &= !GetForceFlagOffset(flag);		\
m_isEmitterForceChanged = true;										\

void BaseParticleUpdateProperty::ApplyGravityForce(const DirectX::XMFLOAT3& gravityForce) noexcept
{
	OnFlag(EForceFlag::Gravity);
	m_emitterForceProperty.gravityForce = gravityForce;
}

void BaseParticleUpdateProperty::RemoveGravityForce() noexcept
{
	OffFlag(EForceFlag::Gravity);
}

void BaseParticleUpdateProperty::ApplyDragForce(float dragCoefficient) noexcept
{
	OnFlag(EForceFlag::Drag);
	m_emitterForceProperty.dragCoefficient = dragCoefficient;
}

void BaseParticleUpdateProperty::RemoveDragForce() noexcept
{
	OffFlag(EForceFlag::Drag);
}

void BaseParticleUpdateProperty::ApplyCurlNoiseForce(float curlNoiseOctave, float curlNoiseCoefficient) noexcept
{
	OnFlag(EForceFlag::CurNoise);
	m_emitterForceProperty.curlNoiseOctave = curlNoiseOctave;
	m_emitterForceProperty.curlNoiseCoefficient = curlNoiseCoefficient;
}

void BaseParticleUpdateProperty::RemoveCurlNoiseForce() noexcept
{
	OffFlag(EForceFlag::CurNoise);
}

void BaseParticleUpdateProperty::AddVortexForce(
	const XMFLOAT3 vortexOrigin, 
	const XMFLOAT3 vortexAxis, 
	float vortexRadius, 
	float vortexTightness
)
{
	UINT vortexCount = GetNForceCount(m_emitterForceProperty.nForceCount, ENForceKind::Vortex);
	if (vortexCount == MaxNForceCount)
	{
		throw exception("Vortex Count Exceed Maximum Value");
	}
	else
	{
		OnFlag(EForceFlag::Vortex);
		IncrementNForceCount(m_emitterForceProperty.nForceCount, ENForceKind::Vortex);
		SVortexForce& vortexForce = m_emitterForceProperty.nVortexForce[vortexCount];
		vortexForce.vortexOrigin = vortexOrigin;
		vortexForce.vortexAxis = vortexAxis;
		vortexForce.vortexRadius = vortexRadius;
		vortexForce.vortexTightness = vortexTightness;
	}
}

void BaseParticleUpdateProperty::RemoveVortexForce(UINT vortexForceIndex)
{
	UINT vortexCount = GetNForceCount(m_emitterForceProperty.nForceCount, ENForceKind::Vortex);
	if (vortexCount == 0)
	{
		throw exception("No Vortex To Remove");
	}
	else
	{
		DecrementNForceCount(m_emitterForceProperty.nForceCount, ENForceKind::Vortex);
		if (vortexCount == 1)
		{
			OffFlag(EForceFlag::Vortex);
		}
		else if (vortexCount == MaxNForceCount)
		{
			m_isEmitterForceChanged = true;
		}
		else
		{
			std::memmove(&m_emitterForceProperty.nVortexForce[vortexForceIndex],
				&m_emitterForceProperty.nVortexForce[vortexForceIndex + 1],
				sizeof(SVortexForce) * (MaxNForceCount - (vortexForceIndex + 1))
			);
			m_isEmitterForceChanged = true;
		}
	}
}

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
		OnFlag(EForceFlag::LineInteraction);
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
			OffFlag(EForceFlag::LineInteraction);
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
		OnFlag(EForceFlag::PointInteraction);
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
			OffFlag(EForceFlag::PointInteraction);
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
	bool isVortexSet = GetFlag(EForceFlag::Vortex);
	bool isLineInteractionSet = GetFlag(EForceFlag::LineInteraction);
	bool isPointInteractionSet = GetFlag(EForceFlag::PointInteraction);

	if (!isGravitySet)
	{
		static XMFLOAT3 gravityForce = XMFLOAT3(0.f, 0.f, 0.f);
		DragFloat3("중력 벡터", &gravityForce.x, 0.1f, -1000.f, 1000.f, "%.1f");
		if (Button("중력 추가")) { ApplyGravityForce(gravityForce); }
	}

	if (isGravitySet)
	{
		ImGui::SeparatorText("중력");
		if (DragFloat3("중력 벡터", &m_emitterForceProperty.gravityForce.x, 0.1f, -1000.f, 1000.f, "%.1f"))
		{
			m_isEmitterForceChanged = true;
		}
	}
	else
	{

	}

	if (isDragSet)
	{
		ImGui::SeparatorText("항력");
		if (DragFloat("항력계수", &m_emitterForceProperty.dragCoefficient, 0.1f, 0.f, 1000.f, "%.1f"))
		{
			m_isEmitterForceChanged = true;
		}
	}

	if (GetFlag(EForceFlag::CurNoise))
	{

	}

	if (GetFlag(EForceFlag::Vortex))
	{

	}

	if (GetFlag(EForceFlag::LineInteraction))
	{

	}

	if (GetFlag(EForceFlag::PointInteraction))
	{

	}
}