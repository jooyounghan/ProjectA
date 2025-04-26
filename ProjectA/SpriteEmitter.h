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
		const std::function<void(UINT, const SSpriteInterpInformation&)>& interpInformationChangedHandler
	);
	~SpriteEmitter() override = default;

protected:
	std::function<void(UINT, const SSpriteInterpInformation&)> m_onInterpInformationChanged;

public:
	virtual void CreateProperty() override;
};

