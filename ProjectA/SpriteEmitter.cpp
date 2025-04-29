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
	const std::function<void(UINT, UINT, bool, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterSelectedHandler,
	const std::function<void(UINT, UINT, bool, float, EInterpolationMethod, IInterpolater<4>*)>& gpuColorInterpolaterUpdatedHandler,
	const std::function<void(UINT, UINT, bool, EInterpolationMethod, IInterpolater<2>*)>& gpuSpriteSizeInterpolaterSelectedHandler,
	const std::function<void(UINT, UINT, bool, float, EInterpolationMethod, IInterpolater<2>*)>& gpuSpriteSizeInterpolaterUpdatedHandler,
	const std::function<void(UINT, UINT, bool, EInterpolationMethod, IInterpolater<1>*)>& gpuSpriteIndexInterpolaterSelectedHandler,
	const std::function<void(UINT, UINT, bool, float, EInterpolationMethod, IInterpolater<1>*)>& gpuSpriteIndexInterpolaterUpdatedHandler
)
	: AEmitter(
		static_cast<UINT>(EEmitterType::SpriteEmitter),
		emitterID, position, angle,
		worldTransformChangedHandler,
		forcePropertyChangedHandler,
		gpuColorInterpolaterSelectedHandler,
		gpuColorInterpolaterUpdatedHandler
	),
	m_spriteSizeInterpolaterID(InterpPropertyNotSelect),
	m_onSpriteSizeInterpolaterSelected(gpuSpriteSizeInterpolaterSelectedHandler),
	m_onSpriteSizeInterpolaterUpdated(gpuSpriteSizeInterpolaterUpdatedHandler),
	m_spriteIndexInterpolaterID(InterpPropertyNotSelect),
	m_onSpriteIndexInterpolaterSelected(gpuSpriteIndexInterpolaterSelectedHandler),
	m_onSpriteIndexInterpolaterUpdated(gpuSpriteIndexInterpolaterUpdatedHandler)
{
}


void SpriteEmitter::CreateProperty()
{
	m_initialSpawnProperty = make_unique<CInitialSpawnProperty>();
	m_emitterUpdateProperty = make_unique<CEmitterUpdateProperty>();
	m_runtimeSpawnProperty = make_unique<SpriteSpawnProperty>(
		[this](bool isColorGPUInterpolaterOn, EInterpolationMethod colorIntperpolationMethod, IInterpolater<4>* colorInterpolater) 
		{ 
			m_onGpuColorInterpolaterSelected(GetEmitterID(), m_colorInterpolaterID, isColorGPUInterpolaterOn, colorIntperpolationMethod, colorInterpolater);
		},
		[this](bool isColorGPUInterpolaterOn, float maxLife, EInterpolationMethod colorIntperpolationMethod, IInterpolater<4>* colorInterpolater)
		{
			m_onGpuColorInterpolaterUpdated(GetEmitterID(), m_colorInterpolaterID, isColorGPUInterpolaterOn, maxLife, colorIntperpolationMethod, colorInterpolater);
		},
		[this](bool isSpriteSizeGPUInterpolaterOn, EInterpolationMethod spriteSizeIntperpolationMethod, IInterpolater<2>* spriteSizeInterpolater)
		{
			m_onSpriteSizeInterpolaterSelected(GetEmitterID(), m_spriteSizeInterpolaterID, isSpriteSizeGPUInterpolaterOn, spriteSizeIntperpolationMethod, spriteSizeInterpolater);
		},
		[this](bool isSpriteSizeGPUInterpolaterOn, float maxLife, EInterpolationMethod spriteSizeIntperpolationMethod, IInterpolater<2>* spriteSizeInterpolater) 
		{ 
			m_onSpriteSizeInterpolaterUpdated(GetEmitterID(), m_spriteSizeInterpolaterID, isSpriteSizeGPUInterpolaterOn, maxLife, spriteSizeIntperpolationMethod, spriteSizeInterpolater);
		},
		[this](bool isSpriteIndexGPUInterpolaterOn, EInterpolationMethod spriteIndexIntperpolationMethod, IInterpolater<1>* spriteIndexInterpolater)
		{
			m_onSpriteIndexInterpolaterSelected(GetEmitterID(), m_spriteIndexInterpolaterID, isSpriteIndexGPUInterpolaterOn, spriteIndexIntperpolationMethod, spriteIndexInterpolater);
		},
			[this](bool isSpriteIndexGPUInterpolaterOn, float maxLife, EInterpolationMethod spriteIndexIntperpolationMethod, IInterpolater<1>* spriteIndexInterpolater)
		{
			m_onSpriteIndexInterpolaterUpdated(GetEmitterID(), m_spriteIndexInterpolaterID, isSpriteIndexGPUInterpolaterOn, maxLife, spriteIndexIntperpolationMethod, spriteIndexInterpolater);
		}
	);
	m_forceUpdateProperty = make_unique<ForceUpdateProperty>(
		[this](const SEmitterForceProperty& forceProperty) { m_onForcePropertyChanged(GetEmitterID(), forceProperty); }
	);

}
