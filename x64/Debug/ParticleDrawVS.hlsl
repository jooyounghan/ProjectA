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
	float4 currentWorldVelocity = float4(currentPoint.velocity, 0.f);
	float4 prevWorldPos = currentWorldPos - dt * currentWorldVelocity;
	
	float4 currentClippedPos = mul(currentWorldPos, viewProjMatrix);
    result.viewPos = currentClippedPos;
	result.color = currentPoint.color;
	result.xyScale = float2(currentPoint.xyScale);

#ifdef SPRITE_EMITTER
	result.spriteIndex = currentPoint.spriteIndex;
	result.emitterID = currentPoint.emitterID;
#else
	float4 prevClippedPos = mul(prevWorldPos, viewProjMatrix);
	currentClippedPos /= currentClippedPos.w;
	prevClippedPos /= prevClippedPos.w;
	float4 viewVelocity = currentClippedPos - prevClippedPos;	
	result.velocity = viewVelocity.xy;
#endif

	return result;
}