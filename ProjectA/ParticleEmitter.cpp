#include "ParticleEmitter.h"
#include "InitialSpawnProperty.h"
#include "EmitterUpdateProperty.h"
#include "ParticleSpawnProperty.h"
#include "ForceUpdateProperty.h"

#include "EmitterTypeDefinition.h"

using namespace std;
using namespace DirectX;

ParticleEmitter::ParticleEmitter(
	UINT emitterID, 
	const XMVECTOR& position, 
	const XMVECTOR& angle,
	const function<void(UINT, const XMMATRIX&)>& worldTransformChangedHandler,
	const function<void(UINT, const SEmitterForceProperty&)>& forcePropertyChangedHandler,
	const std::function<void(UINT, UINT, bool, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterSelectedHandler,
	const std::function<void(UINT, UINT, bool, float, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterUpdatedHandler
)
	: AEmitter(
		static_cast<UINT>(EEmitterType::ParticleEmitter),
		emitterID, position, angle,
		worldTransformChangedHandler,
		forcePropertyChangedHandler,
		gpuColorInterpolaterSelectedHandler,
		gpuColorInterpolaterUpdatedHandler
	)
{

}

void ParticleEmitter::CreateProperty()
{
	m_initialSpawnProperty = make_unique<CInitialSpawnProperty>();
	m_emitterUpdateProperty = make_unique<CEmitterUpdateProperty>();
	m_runtimeSpawnProperty = make_unique<ParticleSpawnProperty>(
		[this](bool isColorGPUInterpolaterOn, EInterpolationMethod colorIntperpolationMethod, IInterpolater<4>* colorInterpolater)
		{
			m_onGpuColorInterpolaterSelected(GetEmitterID(), m_colorInterpolaterID, isColorGPUInterpolaterOn, colorIntperpolationMethod, colorInterpolater);
		},
		[this](bool isColorGPUInterpolaterOn,  float maxLife, EInterpolationMethod colorIntperpolationMethod, IInterpolater<4>* colorInterpolater)
		{ 
			m_onGpuColorInterpolaterUpdated(GetEmitterID(), m_colorInterpolaterID, isColorGPUInterpolaterOn, maxLife, colorIntperpolationMethod, colorInterpolater);
		}
	);
	m_forceUpdateProperty = make_unique<ForceUpdateProperty>(
		[this](const SEmitterForceProperty& forceProperty) { m_onForcePropertyChanged(GetEmitterID(), forceProperty); }
	);
}

