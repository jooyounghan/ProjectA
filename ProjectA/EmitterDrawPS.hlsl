#include "EmitterDrawCommon.hlsli"

float4 main() : SV_TARGET
{
#ifdef PARTICLE_EMITTER
	return float4(1.0f, 1.0f, 0.0f, 1.0f);
#elif defined(RIBBON_EMITTER)
	return float4(0.0f, 1.0f, 1.0f, 1.0f);
#elif defined(SPRITE_EMITTER)
	return float4(1.0f, 0.0f, 1.0f, 1.0f);
#elif defined(MESH_EMITTER)
	return float4(1.0f, 1.0f, 1.0f, 1.0f);
#else
	return float4(0.f, 0.f, 0.f, 1.f);
#endif	
}