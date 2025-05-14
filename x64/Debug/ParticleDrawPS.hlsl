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

	uint   lowerIndex = (uint)floor(input.spriteIndex);
	uint   upperIndex = (uint)ceil(input.spriteIndex);
	float  t          = input.spriteIndex - lowerIndex;

	float2 baseCoord = input.texCoord;
	float2 lowerUV = float2(
		(lowerIndex + baseCoord.x) / spriteTextureCount,
		baseCoord.y
	);

	float2 upperUV = float2(
		(upperIndex + baseCoord.x) / spriteTextureCount,
		baseCoord.y
	);

	float4 lowerSample = spriteImage.Sample(wrapSampler, float3(lowerUV, emitterID));
	float4 upperSample = spriteImage.Sample(wrapSampler, float3(upperUV, emitterID));
	float4 spriteColor = lerp(lowerSample, upperSample, t);
	float4 color = input.color * spriteColor;
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

#else
	ParticlePSOut result;
	result.f4ShotFilm = color * 100.f;
	result.f4BlurFilm = color;
#endif
	return result;

}