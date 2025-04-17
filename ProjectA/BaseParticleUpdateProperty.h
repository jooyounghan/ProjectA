#pragma once
#include "IProperty.h"
#include <DirectXMath.h>

enum class EForceFlag
{
	Gravity,
	Drag,
	CurNoise,
	Vortex,
	LineInteraction,
	PointInteraction,
};

constexpr UINT ceil_log2(UINT n)
{
	return (n <= 1) ? 0 : 1 + ceil_log2((n + 1) >> 1);
}

constexpr UINT MaxNForceCount = 4;

enum class ENForceKind
{
	Vortex,
	LineInteraction,
	PointInteraction,
	ENForceKindCount
};

static_assert(ceil_log2(MaxNForceCount) * static_cast<UINT>(ENForceKind::ENForceKindCount) <= 32);

constexpr UINT GetForceFlagOffset(EForceFlag forceFlag);
constexpr UINT GetNForceCount(UINT nForceCount, ENForceKind forceKind);
constexpr UINT SetNForceCount(UINT nForceCount, ENForceKind forceKind, UINT newValue);
constexpr UINT IncrementNForceCount(UINT nForceCount, ENForceKind forceKind);
constexpr UINT DecrementNForceCount(UINT nForceCount, ENForceKind forceKind);

struct SVortexForce
{
	DirectX::XMFLOAT3 vortexOrigin;
	DirectX::XMFLOAT3 vortexAxis;
	float vortexRadius;
	float vortextCoefficient;
	float vortexTightness;
};

struct SLineInteractionForce
{
	DirectX::XMFLOAT3 lineInteractionOrigin;
	DirectX::XMFLOAT3 lineInteractionAxis;
	float interactionDistance;
	float interactionCoefficient;
};

struct SPointInteractionForce
{
	DirectX::XMFLOAT3 pointInteractionCenter;
	float interactionRadius;
	float interactionCoefficient;
};

struct SEmitterForceProperty
{
	UINT forceFlag;
	DirectX::XMFLOAT3 gravityForce;
	float dragCoefficient;
	float curlNoiseOctave;
	float curlNoiseCoefficient;
	UINT nForceCount;
	SVortexForce nVortexForce[MaxNForceCount];
	SLineInteractionForce nLineInteractionForce[MaxNForceCount];
	SPointInteractionForce nPointInteractionForce[MaxNForceCount];
};

class BaseParticleUpdateProperty : public IProperty
{
public:
	BaseParticleUpdateProperty(
		bool& isEmitterForceChanged,
		SEmitterForceProperty& emitterForceProperty
	);
	virtual ~BaseParticleUpdateProperty() = default;

protected:
	bool& m_isEmitterForceChanged;
	SEmitterForceProperty& m_emitterForceProperty;

public:
	void ApplyGravityForce(const DirectX::XMFLOAT3& gravityForce) noexcept;
	void RemoveGravityForce() noexcept;

public:
	void ApplyDragForce(float dragCoefficient) noexcept;
	void RemoveDragForce() noexcept;

public:
	void ApplyCurlNoiseForce(float curlNoiseOctave, float curlNoiseCoefficient) noexcept;
	void RemoveCurlNoiseForce() noexcept;

public:
	void AddVortexForce(
		const DirectX::XMFLOAT3 vortexOrigin, 
		const DirectX::XMFLOAT3 vortexAxis, 
		float vortexRadius, 
		float vortexTightness
	);
	void RemoveVortexForce(UINT vortexForceIndex);

public:
	void AddLineInteractionForce(
		const DirectX::XMFLOAT3 lineInteractionOrigin,
		const DirectX::XMFLOAT3 lineInteractionAxis,
		float interactionDistance,
		float interactionCoefficient
	);
	void RemoveLineInteractionForce(UINT lineInteractionForceIndex);

public:
	void AddPointInteractionForce(
		DirectX::XMFLOAT3 pointInteractionCenter,
		float interactionRadius,
		float interactionCoefficient
	);
	void RemovePointInteractionForce(UINT pointInteractionForceIndex);

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

public:
	virtual void DrawPropertyUI() override;
};

