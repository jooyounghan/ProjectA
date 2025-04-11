#include "SimulateCommon.hlsli"

[numthreads(LocalThreadCount, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint index = DTid.x;
	if (index < Pcurrent)
	{
		uint particleIndex = currentIndices[index];
		Particle currentParticle = particles[particleIndex];

		float density = currentParticle.density;
		// 가속도 계산
		currentParticle.accelerate = float3(0.f, -9.8 * (density - 1.f), 0.f);

		// density dissipation
		density -= 0.001f;
		currentParticle.density = density;

		// 가속도를 통한 적분
		currentParticle.velocity += currentParticle.accelerate * dt;
		currentParticle.worldPos += currentParticle.velocity * dt;

		particles[particleIndex] = currentParticle;
	}
}