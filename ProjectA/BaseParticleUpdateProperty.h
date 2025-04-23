#pragma once
#include "IProperty.h"
#include "EmitterForceProperty.h"

#include <functional>
#include <string>

enum class EForceFlag
{
	Gravity,
	Drag,
	CurNoise,
	Vortex,
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
	PointInteraction,
	ENForceKindCount
};

static_assert(GetRadixCount(MaxNForceCount) * static_cast<UINT>(ENForceKind::ENForceKindCount) <= 32);
static_assert(MaxNForceCount % 4 == 0, "MaxNForceCount must be a multiple of 4.");

constexpr UINT GetForceFlagOffset(EForceFlag forceFlag);
constexpr UINT GetNForceCount(UINT nForceCount, ENForceKind forceKind);
void SetNForceCount(UINT& nForceCount, ENForceKind forceKind, UINT newValue);
void IncrementNForceCount(UINT& nForceCount, ENForceKind forceKind);
void DecrementNForceCount(UINT& nForceCount, ENForceKind forceKind);

class CBaseParticleUpdateProperty : public IProperty
{
public:
	CBaseParticleUpdateProperty(
		UINT forcePropertyIndex,
		SEmitterForceProperty& emitterForceProperty,
		const std::function<void(UINT)>& emitterForceUpdatedHandler
	);
	~CBaseParticleUpdateProperty() override = default;

protected:
	UINT m_forcePropertyIndex;
	SEmitterForceProperty& m_emitterForceProperty;
	bool m_isEmitterForcePropertyChanged;
	std::function<void(UINT)> m_onEmitterForceUpdated;
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
		const std::function<bool()>& handler
	);
	void HandleNForce(
		const std::string& forceName,
		EForceFlag force, 
		ENForceKind nForceKind, 
		const std::function<void(UINT)>& addButtonHandler,
		const std::function<void(UINT)>& deleteButtonHandler,
		const std::function<bool(UINT)>& handler
	);
};

