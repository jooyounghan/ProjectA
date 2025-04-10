#include "ParticleDrawCommon.hlsli"

ParticleVSOut main(uint vertexID : SV_VertexID)
{
	ParticleVSOut result;
	uint index = currentIndices[vertexID];
 	Particle currentPoint = particles[index];
	result.worldPos = currentPoint.worldPos;
	result.viewPos = mul(float4(currentPoint.worldPos, 1.f), viewProjMatrix);
	result.life = currentPoint.life;
	return result;
}