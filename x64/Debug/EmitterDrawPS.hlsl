#include "EmitterDrawCommon.hlsli"

EmitterPSOutput main(EmitterVSOutput input) : SV_TARGET
{
	EmitterPSOutput result;

#ifdef PARTICLE_EMITTER
	result.color = float4(1.0f, 1.0f, 0.0f, 1.0f);
#elif defined(RIBBON_EMITTER)
	result.color = float4(0.0f, 1.0f, 1.0f, 1.0f);
#elif defined(SPRITE_EMITTER)
	result.color = float4(1.0f, 0.0f, 1.0f, 1.0f);
#elif defined(MESH_EMITTER)
	result.color = float4(1.0f, 1.0f, 1.0f, 1.0f);
#else
	result.color = float4(0.f, 0.f, 0.f, 1.f);
#endif	
	result.normalVec = float4(input.normalVec, 0.f);
	return result;
}