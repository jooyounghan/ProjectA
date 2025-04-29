#include "ParticleDrawCommon.hlsli"

float4 main(ParticleGSOut input) : SV_TARGET
{
#ifdef PARTICLE_EMITTER
	float dist = length(0.5f - input.texCoord) * 2.f;
	float scale = smoothstep(0.f, 1.f, 1.f - dist);
	float4 color = input.color;
	return color * scale;
#elif defined(SPRITE_EMITTER)
	return input.color * float4(input.texCoord, 0.0f, 1.0f);
#else
	return float4(0.f, 0.f, 0.f, 1.f);
#endif	
}