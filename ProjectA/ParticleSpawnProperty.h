#pragma once
#include "ARuntimeSpawnProperty.h"
#include "InterpInformation.h"

#include <functional>

class ParticleSpawnProperty : public ARuntimeSpawnProperty
{
public:
	ParticleSpawnProperty(
		const std::function<void(EInterpolationMethod, bool)>& gpuColorInterpolaterSelectHandler,
		const std::function<void(EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterUpdatedHandler,
		const std::function<void(float, UINT)>& particleInterpInformChangedHandler
	);
	~ParticleSpawnProperty() override = default;

protected:
	std::function<void(float, UINT)> m_onParticleInterpInforChanged;
};

