#pragma once
#include "AEmitter.h"
#include "InterpInformation.h"

class SpriteEmitter : public AEmitter
{
public:
	SpriteEmitter(
		UINT emitterID,
		const DirectX::XMVECTOR& position,
		const DirectX::XMVECTOR& angle,
		const std::function<void(UINT, const DirectX::XMMATRIX&)>& worldTransformChangedHandler,
		const std::function<void(UINT, const SEmitterForceProperty&)>& forcePropertyChangedHandler,
		const std::function<void(UINT, EInterpolationMethod, bool)>& gpuColorInterpolaterSelectedHandler,
		const std::function<void(UINT, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterUpdatedHandler,
		const std::function<void(UINT, EInterpolationMethod, bool)>& gpuSpriteSizeInterpolaterSelectedHandler,
		const std::function<void(UINT, EInterpolationMethod, IInterpolater<2>*)>& gpuSpriteSizeInterpolaterUpdatedHandler,
		const std::function<void(UINT, float, UINT, UINT, UINT, UINT)>& spriteInterpInformationChangedHandler
	);
	~SpriteEmitter() override = default;

protected:
	UINT m_spriteSizeInterpolaterID;

public:
	inline UINT GetSpriteSizeInterpolaterID() const noexcept { return m_spriteSizeInterpolaterID; }
	inline void SetSpriteSizeInterpolaterID(UINT spriteSizeInterpolaterID) noexcept { m_spriteSizeInterpolaterID = spriteSizeInterpolaterID; }


protected:
	std::function<void(UINT, EInterpolationMethod, bool)> m_onSpriteSizeInterpolaterSelected;
	std::function<void(UINT, EInterpolationMethod, IInterpolater<2>*)> m_onSpriteSizeInterpolaterUpdated;
	std::function<void(UINT, float, UINT, UINT, UINT, UINT)> m_onSpriteInterpInformationChanged;

public:
	virtual void CreateProperty() override;
};

