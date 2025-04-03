#include "ParticleDrawCommon.hlsli"

ParticleVSOut main(uint vertexID : SV_VertexID)
{
	ParticleVSOut result;
	
	Particle currentPoint = particles[vertexID];

	float4 worldPos = mul(float4(currentPoint.modelPos, 1.f), toWorldTransform);
	result.viewPos = mul(worldPos, viewProjMatrix);
	result.life = currentPoint.life;
	return result;
}