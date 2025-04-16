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
	SEmitterForceProperty& emitterForceRef
)
	: m_isEmitterForceChangedRef(isEmitterForceChanged),
	m_emitterPropertyRef(emitterForceRef)
{
	sizeof(SEmitterForceProperty);
}

#define OnFlag(flag)													\
m_emitterPropertyRef.forceFlag |= GetForceFlagOffset(flag);				\
m_isEmitterForceChangedRef = true;										\

#define OffFlag(flag)													\
m_emitterPropertyRef.forceFlag &= !GetForceFlagOffset(flag);			\
m_isEmitterForceChangedRef = true;										\



void BaseParticleUpdateProperty::ApplyGravityForce(const DirectX::XMFLOAT3& gravityForce) noexcept
{
	OnFlag(EForceFlag::Gravity);
	m_emitterPropertyRef.gravityForce = gravityForce;
}

void BaseParticleUpdateProperty::RemoveGravityForce() noexcept
{
	OffFlag(EForceFlag::Gravity);
}

void BaseParticleUpdateProperty::ApplyDragForce(float dragCoefficient) noexcept
{
	OnFlag(EForceFlag::Drag);
	m_emitterPropertyRef.dragCoefficient = dragCoefficient;
}

void BaseParticleUpdateProperty::RemoveDragForce() noexcept
{
	OffFlag(EForceFlag::Drag);
}

void BaseParticleUpdateProperty::ApplyCurlNoiseForce(float curlNoiseOctave, float curlNoiseCoefficient) noexcept
{
	OnFlag(EForceFlag::CurNoise);
	m_emitterPropertyRef.curlNoiseOctave = curlNoiseOctave;
	m_emitterPropertyRef.curlNoiseCoefficient = curlNoiseCoefficient;
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
	UINT vortexCount = GetNForceCount(m_emitterPropertyRef.nForceCount, ENForceKind::Vortex);
	if (vortexCount == MaxNForceCount)
	{
		throw exception("Vortex Count Exceed Maximum Value");
	}
	else
	{
		OnFlag(EForceFlag::Vortex);
		IncrementNForceCount(m_emitterPropertyRef.nForceCount, ENForceKind::Vortex);
		SVortexForce& vortexForce = m_emitterPropertyRef.nVortexForce[vortexCount];
		vortexForce.vortexOrigin = vortexOrigin;
		vortexForce.vortexAxis = vortexAxis;
		vortexForce.vortexRadius = vortexRadius;
		vortexForce.vortexTightness = vortexTightness;
	}
}

void BaseParticleUpdateProperty::RemoveVortexForce(UINT vortexForceIndex)
{
	UINT vortexCount = GetNForceCount(m_emitterPropertyRef.nForceCount, ENForceKind::Vortex);
	if (vortexCount == 0)
	{
		throw exception("No Vortex To Remove");
	}
	else
	{
		DecrementNForceCount(m_emitterPropertyRef.nForceCount, ENForceKind::Vortex);
		if (vortexCount == 1)
		{
			OffFlag(EForceFlag::Vortex);
		}
		else if (vortexCount == MaxNForceCount)
		{
			m_isEmitterForceChangedRef = true;
		}
		else
		{
			std::memmove(&m_emitterPropertyRef.nVortexForce[vortexForceIndex],
				&m_emitterPropertyRef.nVortexForce[vortexForceIndex + 1],
				sizeof(SVortexForce) * (MaxNForceCount - (vortexForceIndex + 1))
			);
			m_isEmitterForceChangedRef = true;
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
	UINT lineInteractionCount = GetNForceCount(m_emitterPropertyRef.nForceCount, ENForceKind::LineInteraction);
	if (lineInteractionCount == MaxNForceCount)
	{
		throw exception("Line Interaction Count Exceed Maximum Value");
	}
	else
	{
		OnFlag(EForceFlag::LineInteraction);
		IncrementNForceCount(m_emitterPropertyRef.nForceCount, ENForceKind::LineInteraction);
		SLineInteractionForce& lineInteractionForce = m_emitterPropertyRef.nLineInteractionForce[lineInteractionCount];
		lineInteractionForce.lineInteractionOrigin = lineInteractionOrigin;
		lineInteractionForce.lineInteractionAxis = lineInteractionAxis;
		lineInteractionForce.interactionDistance = interactionDistance;
		lineInteractionForce.interactionCoefficient = interactionCoefficient;
	}
}

void BaseParticleUpdateProperty::RemoveLineInteractionForce(UINT lineInteractionForceIndex)
{
	UINT lineInteractionCount = GetNForceCount(m_emitterPropertyRef.nForceCount, ENForceKind::LineInteraction);
	if (lineInteractionCount == 0)
	{
		throw exception("No Line Interaction To Remove");
	}
	else
	{
		DecrementNForceCount(m_emitterPropertyRef.nForceCount, ENForceKind::LineInteraction);
		if (lineInteractionCount == 1)
		{
			OffFlag(EForceFlag::LineInteraction);
		}
		else if (lineInteractionCount == MaxNForceCount)
		{
			m_isEmitterForceChangedRef = true;
		}
		else
		{
			std::memmove(&m_emitterPropertyRef.nLineInteractionForce[lineInteractionForceIndex],
				&m_emitterPropertyRef.nLineInteractionForce[lineInteractionForceIndex + 1],
				sizeof(SLineInteractionForce) * (MaxNForceCount - (lineInteractionForceIndex + 1))
			);
			m_isEmitterForceChangedRef = true;
		}
	}
}

void BaseParticleUpdateProperty::AddPointInteractionForce(
	XMFLOAT3 pointInteractionCenter,
	float interactionRadius,
	float interactionCoefficient
)
{
	UINT pointInteractionCount = GetNForceCount(m_emitterPropertyRef.nForceCount, ENForceKind::PointInteraction);
	if (pointInteractionCount == MaxNForceCount)
	{
		throw exception("Point Interaction Count Exceed Maximum Value");
	}
	else
	{
		OnFlag(EForceFlag::PointInteraction);
		IncrementNForceCount(m_emitterPropertyRef.nForceCount, ENForceKind::PointInteraction);
		SPointInteractionForce& pointInteractionForce = m_emitterPropertyRef.nPointInteractionForce[pointInteractionCount];
		pointInteractionForce.pointInteractionCenter = pointInteractionCenter;
		pointInteractionForce.interactionRadius = interactionRadius;
		pointInteractionForce.interactionCoefficient = interactionCoefficient;
	}
}

void BaseParticleUpdateProperty::RemovePointInteractionForce(UINT pointInteractionForceIndex)
{
	UINT pointInteractionCount = GetNForceCount(m_emitterPropertyRef.nForceCount, ENForceKind::PointInteraction);
	if (pointInteractionCount == 0)
	{
		throw exception("No Point Interaction To Remove");
	}
	else
	{
		DecrementNForceCount(m_emitterPropertyRef.nForceCount, ENForceKind::PointInteraction);
		if (pointInteractionCount == 1)
		{
			OffFlag(EForceFlag::PointInteraction);
		}
		else if (pointInteractionCount == MaxNForceCount)
		{
			m_isEmitterForceChangedRef = true;
		}
		else
		{
			std::memmove(&m_emitterPropertyRef.nPointInteractionForce[pointInteractionForceIndex],
				&m_emitterPropertyRef.nPointInteractionForce[pointInteractionForceIndex + 1],
				sizeof(SPointInteractionForce) * (MaxNForceCount - (pointInteractionForceIndex + 1))
			);
			m_isEmitterForceChangedRef = true;
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

}

std::unique_ptr<BaseParticleUpdateProperty> BaseParticleUpdateProperty::DrawPropertyCreator()
{
	SeparatorText("파티클 업데이트 프로퍼티");
	return nullptr;
}
