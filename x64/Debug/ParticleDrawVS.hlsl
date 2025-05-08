#include "ParticleCommon.hlsli"
#include "ParticleDrawCommon.hlsli"

StructuredBuffer<Particle> particles : register(t0);

#ifdef SPRITE_EMITTER
StructuredBuffer<SpriteAliveIndex> currentIndices : register(t1);
#else
StructuredBuffer<uint> currentIndices : register(t1);
#endif



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

#ifdef SPRITE_EMITTER
    SpriteAliveIndex spriteAliveIndex = currentIndices[vertexID];
	uint index = spriteAliveIndex.index;
#else
    uint index = currentIndices[vertexID];
#endif
    Particle currentPoint = particles[index];
    result.viewPos = mul(float4(currentPoint.worldPos, 1.f), viewProjMatrix);
	result.color = currentPoint.color;
	result.xyScale = float2(currentPoint.xyScale);

#ifdef SPRITE_EMITTER
	result.spriteIndex = currentPoint.spriteIndex;
	result.emitterID = currentPoint.emitterID;
#else
	result.viewVelocity = mul(float4(currentPoint.velocity, 0.f), viewProjMatrix).xyz;
#endif

	return result;
}