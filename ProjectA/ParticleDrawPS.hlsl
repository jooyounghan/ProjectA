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
	uint emitterID = input.emitterID;
	float2 spriteTextureCount = emitterInterpInforms[emitterID].spriteTextureCount;
	uint2 indexer = uint2(spriteTextureCount);


	uint   idx1 = (uint)floor(input.spriteIndex);
	uint   idx2 = (uint)ceil(input.spriteIndex);
	float  t          = input.spriteIndex - idx1;

	uint2 lowerIndex = uint2(idx1 % indexer.x, idx1 / indexer.x);
	uint2 upperIndex = uint2(idx2 % indexer.x, idx2 / indexer.x);

	float2 baseCoord = input.texCoord;
	float2 lowerUV = (lowerIndex + baseCoord) / spriteTextureCount;
	float2 upperUV = (upperIndex + baseCoord) / spriteTextureCount;

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
	float4 emissiveColor = float4(color.xyz * 10.f, color.w);
	result.f4ShotFilm = color;
	result.f4BlurFilm = color;
#endif
	return result;

}