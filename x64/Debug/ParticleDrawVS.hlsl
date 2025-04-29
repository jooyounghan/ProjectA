#include "ParticleDrawCommon.hlsli"

#ifdef SPRITE_EMITTER
SpriteVSOut main(uint vertexID : SV_VertexID)
#else
ParticleVSOut main(uint vertexID : SV_VertexID)
#endif
{
	#ifdef SPRITE_EMITTER
	SpriteVSOut result;
	#else
	ParticleVSOut result;
	#endif

    uint particleIndex = currentIndices[vertexID];
    Particle currentPoint = particles[particleIndex];
    result.viewPos = mul(float4(currentPoint.worldPos, 1.f), viewProjMatrix);
	result.color = currentPoint.color;
	result.xyScale = float2(currentPoint.xyScale);

	#ifdef SPRITE_EMITTER
	result.spriteIndex = currentPoint.spriteIndex;
	#endif

	return result;
}