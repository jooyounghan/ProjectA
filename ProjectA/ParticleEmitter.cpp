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
	const function<void(UINT, EInterpolationMethod, bool)>& gpuColorInterpolaterSelectedHandler,
	const function<void(UINT, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterUpdatedHandler,
	const function<void(UINT, float, UINT, UINT)>& particleInterpInformChangedHandler
)
	: AEmitter(
		static_cast<UINT>(EEmitterType::ParticleEmitter),
		emitterID, position, angle,
		worldTransformChangedHandler,
		forcePropertyChangedHandler,
		gpuColorInterpolaterSelectedHandler,
		gpuColorInterpolaterUpdatedHandler
	),
	m_onParticleInterpInforChanged(particleInterpInformChangedHandler)
{

}

void ParticleEmitter::CreateProperty()
{
	m_initialSpawnProperty = make_unique<CInitialSpawnProperty>();
	m_emitterUpdateProperty = make_unique<CEmitterUpdateProperty>();
	m_runtimeSpawnProperty = make_unique<ParticleSpawnProperty>(
		[this](EInterpolationMethod colorIntperpolationMethod, bool isColorGPUInterpolaterOn) { m_onGpuColorInterpolaterSelected(GetEmitterID(), colorIntperpolationMethod, isColorGPUInterpolaterOn); },
		[this](EInterpolationMethod colorIntperpolationMethod, IInterpolater<4>* colorInterpolater) { m_onGpuColorInterpolaterUpdated(m_colorInterpolaterID, colorIntperpolationMethod, colorInterpolater); },
		[this](float maxLife, UINT interpoalterCofficientCount) { m_onParticleInterpInforChanged(GetEmitterID(), maxLife, m_colorInterpolaterID, interpoalterCofficientCount); }
	);
	m_forceUpdateProperty = make_unique<ForceUpdateProperty>(
		[this](const SEmitterForceProperty& forceProperty) { m_onForcePropertyChanged(GetEmitterID(), forceProperty); }
	);
}

