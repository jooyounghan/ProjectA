#pragma once
#include "IProperty.h"
#include "ISerializable.h"
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
constexpr void SetNForceCount(UINT& nForceCount, ENForceKind forceKind, UINT newValue);
constexpr void IncrementNForceCount(UINT& nForceCount, ENForceKind forceKind);
constexpr void DecrementNForceCount(UINT& nForceCount, ENForceKind forceKind);

class ForceUpdateProperty : public IProperty, public ISerializable
{
public:
	ForceUpdateProperty(
		const std::function<void(const SEmitterForceProperty&)>& emitterForceUpdatedHandler
	);
	~ForceUpdateProperty() override = default;

protected:
	SEmitterForceProperty m_emitterForceProperty;
	bool m_isEmitterForcePropertyChanged;
	std::function<void(const SEmitterForceProperty&)> m_onEmitterForceUpdated;

private:
	inline bool IsForceOn(EForceFlag forceFlag) const noexcept { return (m_emitterForceProperty.forceFlag >> static_cast<UINT>(forceFlag)) & 0b1; }
	void SetFlag(EForceFlag forceFlag, bool isOn);

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;
	virtual void Update(ID3D11DeviceContext* deviceContext, float dt) override;

public:
	virtual void DrawPropertyUI() override;

protected:
	virtual void DrawPropertyUIImpl() override;

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

public:
	virtual void Serialize(std::ofstream& ofs) override;
	virtual void Deserialize(std::ifstream& ifs) override;
};

