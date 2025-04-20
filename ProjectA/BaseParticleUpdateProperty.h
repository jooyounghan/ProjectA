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

constexpr UINT GetRadixCount(UINT n) noexcept
{
	UINT bits = 0;
	while (n > 0) 
	{
		++bits;
		n >>= 1;
	}
	return bits;
}

constexpr UINT MaxNForceCount = 5;

enum class ENForceKind
{
	Vortex,
	LineInteraction,
	PointInteraction,
	ENForceKindCount
};

static_assert(GetRadixCount(MaxNForceCount) * static_cast<UINT>(ENForceKind::ENForceKindCount) <= 32);

constexpr UINT GetForceFlagOffset(EForceFlag forceFlag);
constexpr UINT GetNForceCount(UINT nForceCount, ENForceKind forceKind);
void SetNForceCount(UINT& nForceCount, ENForceKind forceKind, UINT newValue);
void IncrementNForceCount(UINT& nForceCount, ENForceKind forceKind);
void DecrementNForceCount(UINT& nForceCount, ENForceKind forceKind);

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

