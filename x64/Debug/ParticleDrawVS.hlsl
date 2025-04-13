#include "ParticleDrawCommon.hlsli"

ParticleVSOut main(uint vertexID : SV_VertexID)
{
	ParticleVSOut result;
    ParticleSelector particleSelector = currentIndices[vertexID];
    Particle currentPoint = particles[particleSelector.index];
    result.viewPos = float4(currentPoint.worldPos, 1.f);
	result.life = currentPoint.life;

	if (currentPoint.emitterType == 0)
	{
		result.color = float3(0.9f, 0.4f, 0.2f);
	}
	else if (currentPoint.emitterType == 1)
	{
		result.color = float3(0.2f, 0.8f, 0.9f);
	}
	else
	{
		result.color = float3(0.0f, 0.0f, 0.0f);
	}

	return result;
}