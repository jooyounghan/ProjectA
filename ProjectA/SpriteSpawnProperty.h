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
		const std::function<void(bool, float, EInterpolationMethod, IInterpolater<2>*)>& gpuSpriteSizeInterpolaterUpdatedHandler,
		const std::function<void(bool, EInterpolationMethod, IInterpolater<1>*)>& gpuSpriteIndexInterpolaterSelectedHandler,
		const std::function<void(bool, float, UINT, EInterpolationMethod, IInterpolater<1>*)>& gpuSpriteIndexInterpolaterUpdatedHandler
	);
	~SpriteSpawnProperty() override = default;

protected:
	std::function<void(bool, EInterpolationMethod, IInterpolater<2>*)> m_onGpuSpriteSizeInterpolaterSelected;
	std::function<void(bool, float, EInterpolationMethod, IInterpolater<2>*)> m_onGpuSpriteSizeInterpolaterUpdated;

protected:
	std::function<void(bool, EInterpolationMethod, IInterpolater<1>*)> m_onGpuSpriteIndexInterpolaterSelected;
	std::function<void(bool, float, UINT, EInterpolationMethod, IInterpolater<1>*)> m_onGpuSpriteIndexInterpolaterUpdated;

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
	UINT m_spriteTextureCount;

protected:
	SControlPoint<1> m_spriteIndexInitControlPoint;
	SControlPoint<1> m_spriteIndexFinalControlPoint;
	std::vector<SControlPoint<1>> m_spriteIndexControlPoints;
	EInterpolationMethod m_spriteIndexInterpolationMethod;
	std::unique_ptr<IInterpolater<1>> m_spriteIndexInterpolater;

protected:
	bool m_checkGPUSpriteIndexInterpolater;

protected:
	std::unique_ptr<CControlPointGridView<1>> m_spriteIndexControlPointGridView;
	std::unique_ptr<CInterpolaterSelectPlotter<1>> m_spriteIndexInterpolationSelectPlotter;

private:
	void CreateSpriteSizeInterpolaterUI();
	void CreateSpriteIndexInterpolaterUI();

protected:
	virtual void AdjustControlPointsFromLife() override;
	void AdjustControlPointsFromTextureCount();

public:
	virtual void Initialize(ID3D11Device* device, ID3D11DeviceContext* deviceContext) override;

protected:
	virtual void UpdateImpl(ID3D11DeviceContext* deviceContext, float dt);

protected:
	virtual void DrawUIImpl() override;

private:
	void DrawSpriteSizeSetting();
	void DrawSpriteIndexSetting();

public:
	virtual void Serialize(std::ofstream& ofs) override;
	virtual void Deserialize(std::ifstream& ifs) override;
};

