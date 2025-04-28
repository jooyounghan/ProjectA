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
		const std::function<void(UINT, UINT, bool, float, EInterpolationMethod, IInterpolater<2>*)>& gpuSpriteSizeInterpolaterUpdatedHandler
	);
	~SpriteEmitter() override = default;

protected:
	UINT m_spriteSizeInterpolaterID;

public:
	inline UINT GetSpriteSizeInterpolaterID() const noexcept { return m_spriteSizeInterpolaterID; }
	inline void SetSpriteSizeInterpolaterID(UINT spriteSizeInterpolaterID) noexcept { m_spriteSizeInterpolaterID = spriteSizeInterpolaterID; }

protected:
	std::function<void(UINT, UINT, bool, EInterpolationMethod, IInterpolater<2>*)> m_onSpriteSizeInterpolaterSelected;
	std::function<void(UINT, UINT, bool, float, EInterpolationMethod, IInterpolater<2>*)> m_onSpriteSizeInterpolaterUpdated;

public:
	virtual void CreateProperty() override;
};

