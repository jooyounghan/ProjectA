#include "ParticleDrawCommon.hlsli"
#include "InterpolaterHelper.hlsli"

#ifdef SPRITE_EMITTER
StructuredBuffer<SpriteEmitterInterpInform> emitterInterpInforms : register(t0);
Texture2DArray spriteImage : register(t1);
#endif	

SamplerState wrapSampler : register(s0);

#ifdef SPRITE_EMITTER
float4 main(SpriteGSOut input) : SV_TARGET
#else
float4 main(ParticleGSOut input) : SV_TARGET
#endif
{
#ifdef SPRITE_EMITTER
	const uint emitterID = input.emitterID;
	const float spriteTextureCount = emitterInterpInforms[emitterID].spriteTextureCount;

	const uint lowerIndex = floor(input.spriteIndex);
	const uint upperIndex = ceil(input.spriteIndex);
	const float t = input.spriteIndex - lowerIndex;

	float2 originTexCoord = input.texCoord;
	float2 lowerTexCoord = float2(lowerIndex / spriteTextureCount +  originTexCoord.x / spriteTextureCount, originTexCoord.y);
	float2 upperTexCoord = float2(upperIndex / spriteTextureCount + originTexCoord.x / spriteTextureCount , originTexCoord.y);

	// 최종에 들어가기 전에 미리 gamma correction
	float4 lowerSampled = pow(spriteImage.Sample(wrapSampler, float3(lowerTexCoord, emitterID)), 1.0 / 2.2);
	float4 upperSampled = pow(spriteImage.Sample(wrapSampler, float3(upperTexCoord, emitterID)), 1.0 / 2.2);
	float4 spriteSampled = (1.f - t) * lowerSampled + t * upperSampled;

	return input.color * spriteSampled;
#else

	float dist = length(0.5f - input.texCoord) * 2.f;
	float scale = smoothstep(0.f, 1.f, 1.f - dist);
	float4 color = input.color;
	return color * scale;
#endif	
}