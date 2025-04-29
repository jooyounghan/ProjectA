#include "ParticleDrawCommon.hlsli"

#ifdef SPRITE_EMITTER
float4 main(SpriteGSOut input) : SV_TARGET
#else
float4 main(ParticleGSOut input) : SV_TARGET
#endif
{

#ifdef SPRITE_EMITTER
	return input.color * float4(input.texCoord, 20.0f * input.spriteIndex, 1.0f);
#else

	float dist = length(0.5f - input.texCoord) * 2.f;
	float scale = smoothstep(0.f, 1.f, 1.f - dist);
	float4 color = input.color;
	return color * scale;
	return float4(0.f, 0.f, 0.f, 1.f);
#endif	
}