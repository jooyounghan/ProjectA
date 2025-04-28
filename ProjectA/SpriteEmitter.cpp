#include "SpriteEmitter.h"
#include "InitialSpawnProperty.h"
#include "EmitterUpdateProperty.h"
#include "SpriteSpawnProperty.h"
#include "ForceUpdateProperty.h"

#include "EmitterTypeDefinition.h"

using namespace std;
using namespace DirectX;

SpriteEmitter::SpriteEmitter(
	UINT emitterID, 
	const DirectX::XMVECTOR& position, 
	const DirectX::XMVECTOR& angle, 
	const std::function<void(UINT, const DirectX::XMMATRIX&)>& worldTransformChangedHandler,
	const std::function<void(UINT, const SEmitterForceProperty&)>& forcePropertyChangedHandler, 
	const std::function<void(UINT, EInterpolationMethod, bool)>& gpuColorInterpolaterSelectedHandler,
	const std::function<void(UINT, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterUpdatedHandler,
	const std::function<void(UINT, EInterpolationMethod, bool)>& gpuSpriteSizeInterpolaterSelectedHandler,
	const std::function<void(UINT, EInterpolationMethod, IInterpolater<2>*)>& gpuSpriteSizeInterpolaterUpdatedHandler,
	const std::function<void(UINT, float, UINT, UINT, UINT, UINT)>& spriteInterpInformationChangedHandler
)
	: AEmitter(
		static_cast<UINT>(EEmitterType::SpriteEmitter),
		emitterID, position, angle,
		worldTransformChangedHandler,
		forcePropertyChangedHandler,
		gpuColorInterpolaterSelectedHandler,
		gpuColorInterpolaterUpdatedHandler
	),
	m_onSpriteSizeInterpolaterSelected(gpuSpriteSizeInterpolaterSelectedHandler),
	m_onSpriteSizeInterpolaterUpdated(gpuSpriteSizeInterpolaterUpdatedHandler),
	m_onSpriteInterpInformationChanged(spriteInterpInformationChangedHandler)
{
}

void SpriteEmitter::CreateProperty()
{
	m_initialSpawnProperty = make_unique<CInitialSpawnProperty>();
	m_emitterUpdateProperty = make_unique<CEmitterUpdateProperty>();
	m_runtimeSpawnProperty = make_unique<SpriteSpawnProperty>(
		[this](EInterpolationMethod colorIntperpolationMethod, bool isColorGPUInterpolaterOn) { m_onGpuColorInterpolaterSelected(GetEmitterID(), colorIntperpolationMethod, isColorGPUInterpolaterOn); },
		[this](EInterpolationMethod colorIntperpolationMethod, IInterpolater<4>* colorInterpolater) { m_onGpuColorInterpolaterUpdated(m_colorInterpolaterID, colorIntperpolationMethod, colorInterpolater); },
		[this](EInterpolationMethod spriteSizeIntperpolationMethod, bool isSpriteSizeGPUInterpolaterOn) { m_onSpriteSizeInterpolaterSelected(GetEmitterID(), spriteSizeIntperpolationMethod, isSpriteSizeGPUInterpolaterOn); },
		[this](EInterpolationMethod spriteSizeIntperpolationMethod, IInterpolater<2>* spriteSizeInterpolater) { m_onSpriteSizeInterpolaterUpdated(m_colorInterpolaterID, spriteSizeIntperpolationMethod, spriteSizeInterpolater); },
		[this](float maxLife, UINT colorInterpoalterCofficientCount, UINT spriteSizeInterpoalterCofficientCount) { m_onSpriteInterpInformationChanged(GetEmitterID(), maxLife, m_colorInterpolaterID, colorInterpoalterCofficientCount, m_spriteSizeInterpolaterID, spriteSizeInterpoalterCofficientCount); }
	);
	m_forceUpdateProperty = make_unique<ForceUpdateProperty>(
		[this](const SEmitterForceProperty& forceProperty) { m_onForcePropertyChanged(GetEmitterID(), forceProperty); }
	);

}
