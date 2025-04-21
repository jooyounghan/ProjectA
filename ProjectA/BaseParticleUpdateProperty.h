#pragma once
#include "IProperty.h"
#include "DefineLinkedWithShader.h"
#include <DirectXMath.h>
#include <functional>

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

private:
	inline bool IsForceOn(EForceFlag forceFlag) const noexcept { return (m_emitterForceProperty.forceFlag >> static_cast<UINT>(forceFlag)) & 0b1; }
	void SetFlag(EForceFlag forceFlag, bool isOn);

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

public:
	virtual void DrawPropertyUI() override;

private:
	void HandleSingleForce(
		const std::string& forceName, 
		EForceFlag force, 
		const std::function<void()>& handler
	);
	void HandleNForce(
		const std::string& forceName,
		EForceFlag force, 
		ENForceKind nForceKind, 
		const std::function<void(UINT)>& addButtonHandler,
		const std::function<void(UINT)>& deleteButtonHandler,
		const std::function<void(UINT)>& handler
	);
};

