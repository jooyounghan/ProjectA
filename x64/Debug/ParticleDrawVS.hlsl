#include "ParticleDrawCommon.hlsli"

ParticleVSOut main(uint vertexID : SV_VertexID)
{
	ParticleVSOut result;
	
	Particle currentPoint = particles[vertexID];
	result.viewPos = mul(float4(currentPoint.worldPos, 1.f), viewProjMatrix);
	result.life = currentPoint.life;
	return result;
}