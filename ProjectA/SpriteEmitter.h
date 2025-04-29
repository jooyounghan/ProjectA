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
		const std::function<void(UINT, UINT, bool, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterSelectedHandler,
		const std::function<void(UINT, UINT, bool, float, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterUpdatedHandler,
		const std::function<void(UINT, UINT, bool, EInterpolationMethod, IInterpolater<2>*)>& gpuSpriteSizeInterpolaterSelectedHandler,
		const std::function<void(UINT, UINT, bool, float, EInterpolationMethod, IInterpolater<2>*)>& gpuSpriteSizeInterpolaterUpdatedHandler,
		const std::function<void(UINT, UINT, bool, EInterpolationMethod, IInterpolater<1>*)>& gpuSpriteIndexInterpolaterSelectedHandler,
		const std::function<void(UINT, UINT, bool, float, EInterpolationMethod, IInterpolater<1>*)>& gpuSpriteIndexInterpolaterUpdatedHandler
	);
	~SpriteEmitter() override = default;

protected:
	UINT m_spriteSizeInterpolaterID;
	std::function<void(UINT, UINT, bool, EInterpolationMethod, IInterpolater<2>*)> m_onSpriteSizeInterpolaterSelected;
	std::function<void(UINT, UINT, bool, float, EInterpolationMethod, IInterpolater<2>*)> m_onSpriteSizeInterpolaterUpdated;

protected:
	UINT m_spriteIndexInterpolaterID;
	std::function<void(UINT, UINT, bool, EInterpolationMethod, IInterpolater<1>*)> m_onSpriteIndexInterpolaterSelected;
	std::function<void(UINT, UINT, bool, float, EInterpolationMethod, IInterpolater<1>*)> m_onSpriteIndexInterpolaterUpdated;

public:
	inline void SetSpriteSizeInterpolaterID(UINT spriteSizeInterpolaterID) noexcept { m_spriteSizeInterpolaterID = spriteSizeInterpolaterID; }
	inline void SetSpriteIndexInterpolaterID(UINT spriteIndexInterpolaterID) noexcept { m_spriteIndexInterpolaterID = spriteIndexInterpolaterID; }

public:
	virtual void CreateProperty() override;
};

