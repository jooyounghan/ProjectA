#include "ParticleDrawCommon.hlsli"

ParticleVSOut main(uint vertexID : SV_VertexID)
{
	ParticleVSOut result;

    uint particleIndex = currentIndices[vertexID];
    Particle currentPoint = particles[particleIndex];
    result.viewPos = mul(float4(currentPoint.worldPos, 1.f), viewProjMatrix);
	result.color = currentPoint.color;
	result.xyScale = float2(currentPoint.xyScale);

	return result;
}