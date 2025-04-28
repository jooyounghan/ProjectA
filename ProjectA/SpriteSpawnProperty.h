#pragma once
#include "ARuntimeSpawnProperty.h"
#include "InterpInformation.h"

#include <functional>

class SpriteSpawnProperty : public ARuntimeSpawnProperty
{
public:
	SpriteSpawnProperty(
		const std::function<void(EInterpolationMethod, bool)>& gpuColorInterpolaterSelectHandler,
		const std::function<void(EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterUpdatedHandler,
		const std::function<void(EInterpolationMethod, bool)>& gpuSpriteSizeInterpolaterSelectedHandler,
		const std::function<void(EInterpolationMethod, IInterpolater<2>*)>& gpuSpriteSizeInterpolaterUpdatedHandler,
		const std::function<void(float, UINT, UINT)>& spriteInterpInformationChangedHandler
	);
	~SpriteSpawnProperty() override = default;

protected:
	std::function<void(EInterpolationMethod, bool)> m_onGpuSpriteSizeInterpolaterSelected;
	std::function<void(EInterpolationMethod, IInterpolater<2>*)> m_onGpuSpriteSizeInterpolaterUpdated;
	std::function<void(float, UINT, UINT)> m_onSpriteInterpInformationChanged;

protected:
	SSpriteInterpInformation m_spriteSizeInterpInformation;

protected:
	SControlPoint<2> m_spriteSizeInitControlPoint;
	SControlPoint<2> m_spriteSizeFinalControlPoint;
	std::vector<SControlPoint<2>> m_spriteSizeControlPoints;
	EInterpolationMethod m_spriteSizeInterpolationMethod;
	std::unique_ptr<IInterpolater<2>> m_spriteSizeInterpolater;

protected:
	bool m_checkGPUSpriteSizeInterpolater;

protected:
	std::unique_ptr<CControlPointGridView<2>> m_spriteSizeControlPointGridView;
	std::unique_ptr<CInterpolaterSelectPlotter<2>> m_spriteSizeInterpolationSelectPlotter;

protected:
	virtual void AdjustControlPointsFromLife() override;

protected:
	virtual void DrawPropertyUIImpl() override;
};

