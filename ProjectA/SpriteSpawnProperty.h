#pragma once
#include "ARuntimeSpawnProperty.h"
#include "InterpInformation.h"

#include <functional>

class SpriteSpawnProperty : public ARuntimeSpawnProperty
{
public:
	SpriteSpawnProperty(
		uint32_t maxEmitterCount,
		const std::function<void(const SSpriteInterpInformation&)>& spriteInterpInformationChangedHandler
	);
	~SpriteSpawnProperty() override = default;

protected:
	std::function<void(const SSpriteInterpInformation&)> m_onSpriteInterpInformationChanged;

protected:
	SSpriteInterpInformation m_spriteSizeInterpInformation;

protected:
	SControlPoint<2> m_spriteSizeInitControlPoint;
	SControlPoint<2> m_spriteSizeFinalControlPoint;
	std::vector<SControlPoint<2>> m_spriteSizeControlPoints;
	EInterpolationMethod m_spriteSizeInterpolationMethod;
	std::unique_ptr<IInterpolater<2>> m_spriteSizeInterpolater;

protected:
	std::unique_ptr<CGPUInterpPropertyManager<2, 2>> m_d1Dim2PorpertyManager;
	std::unique_ptr<CGPUInterpPropertyManager<2, 4>> m_d3Dim2PorpertyManager;

protected:
	bool m_checkGPUSpriteSizeInterpolater;

protected:
	std::unique_ptr<CControlPointGridView<2>> m_spriteSizeControlPointGridView;
	std::unique_ptr<CInterpolaterSelectPlotter<2>> m_spriteSizeInterpolationSelectPlotter;

protected:
	virtual void AdjustControlPointsFromLife() override;
	virtual void OnInterpolateInformationChagned() override;

protected:
	virtual void DrawPropertyUIImpl() override;
};

