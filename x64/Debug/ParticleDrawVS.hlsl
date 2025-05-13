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
    SpriteAliveIndex spriteAliveIndex = currentIndices[vertexID];
	uint index = spriteAliveIndex.index;
#else
	ParticleVSOut result;
    uint index = currentIndices[vertexID];
#endif

    Particle currentPoint = particles[index];

	float4 currentWorldPos = float4(currentPoint.worldPos, 1.f);
	float4 currentClippedPos = mul(currentWorldPos, viewProjMatrix);

    result.viewPos = currentClippedPos;
	result.color = currentPoint.color;
	result.xyScale = float2(currentPoint.xyScale);

#ifdef SPRITE_EMITTER
	result.spriteIndex = currentPoint.spriteIndex;
	result.emitterID = currentPoint.emitterID;
#else
	float4 currentVelocity = float4(currentPoint.velocity, 0.f);
	float4 currentClippedVelocity =  mul(currentVelocity, viewProjMatrix);
	result.velocity = currentClippedVelocity.xyz / currentClippedVelocity.w;
#endif

	return result;
}