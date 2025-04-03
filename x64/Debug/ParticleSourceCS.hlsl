#include "ParticleCommon.hlsli"

RWStructuredBuffer<Particle> particles : register(u0);

[numthreads(64, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint particleIdx = DTid.x;
	if (particleIdx < 32 * 32 * 32)
	{
		Particle currentParticle = particles[particleIdx];
		currentParticle.life -= dt * 0.1f;
		particles[particleIdx] = currentParticle;
	}
}