#pragma once
#include "ARuntimeSpawnProperty.h"
#include "InterpInformation.h"

#include <functional>

class SpriteSpawnProperty : public ARuntimeSpawnProperty
{
public:
	SpriteSpawnProperty(
		const std::function<void(bool, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterSelectedHandler,
		const std::function<void(bool, float, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterUpdatedHandler,
		const std::function<void(bool, EInterpolationMethod, IInterpolater<2>*)>& gpuSpriteSizeInterpolaterSelectedHandler,
		const std::function<void(bool, float, EInterpolationMethod, IInterpolater<2>*)>& gpuSpriteSizeInterpolaterUpdatedHandler
	);
	~SpriteSpawnProperty() override = default;

protected:
	std::function<void(bool, EInterpolationMethod, IInterpolater<2>*)> m_onGpuSpriteSizeInterpolaterSelected;
	std::function<void(bool, float, EInterpolationMethod, IInterpolater<2>*)> m_onGpuSpriteSizeInterpolaterUpdated;

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


public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;

protected:
	virtual void DrawPropertyUIImpl() override;

public:
	virtual void Serialize(std::ofstream& ofs) override;
	virtual void Deserialize(std::ifstream& ifs) override;
};

