#pragma once
#include "RuntimeSpawnProperty.h"
#include "InterpInformation.h"

#include <functional>

class CParticleRuntimeSpawnProperty : public CRuntimeSpawnProperty
{
public:
	CParticleRuntimeSpawnProperty(
		const std::function<void(bool, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterSelectedHandler,
		const std::function<void(bool, float, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterUpdatedHandler
	);
	~CParticleRuntimeSpawnProperty() override = default;

protected:
	virtual void DrawUIImpl() override;
};

