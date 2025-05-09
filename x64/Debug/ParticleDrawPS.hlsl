#include "ParticleDrawCommon.hlsli"
#include "InterpolaterHelper.hlsli"

#ifdef SPRITE_EMITTER
StructuredBuffer<SpriteEmitterInterpInform> emitterInterpInforms : register(t0);
Texture2DArray spriteImage : register(t1);
#endif	

SamplerState wrapSampler : register(s0);

#ifdef SPRITE_EMITTER
SpritePSOut main(SpriteGSOut input) : SV_TARGET
#else
ParticlePSOut main(ParticleGSOut input) : SV_TARGET
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

	float4 lowerSampled = spriteImage.Sample(wrapSampler, float3(lowerTexCoord, emitterID));
	float4 upperSampled = spriteImage.Sample(wrapSampler, float3(upperTexCoord, emitterID));
	float4 spriteSampled = (1.f - t) * lowerSampled + t * upperSampled;
	float4 color = input.color * spriteSampled;
#else
    float3 offsetTexCoord = float3((input.texCoord - 0.5f) * 2.f, 0.f);
    offsetTexCoord.y *= -1.f;

	float dist = length(offsetTexCoord);

	float scale = smoothstep(0.f, 1.f, 1.f - dist);
	float4 color = input.color * scale;
#endif	

#ifdef SPRITE_EMITTER
	SpritePSOut result;
	result.f4ShotFilm = color;
	result.f4BlurFilm = color;
#else
	ParticlePSOut result;
	result.f4ShotFilm = color;
	result.f4BlurFilm = color;
	result.f4MotionVector = float4(input.velocity, 0.f, 1.f);
#endif
	return result;

}