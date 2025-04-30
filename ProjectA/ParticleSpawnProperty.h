#pragma once
#include "ARuntimeSpawnProperty.h"
#include "InterpInformation.h"

#include <functional>

class ParticleSpawnProperty : public ARuntimeSpawnProperty
{
public:
	ParticleSpawnProperty(
		const std::function<void(bool, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterSelectedHandler,
		const std::function<void(bool, float, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterUpdatedHandler
	);
	~ParticleSpawnProperty() override = default;

protected:
	virtual void DrawUIImpl() override;
};

