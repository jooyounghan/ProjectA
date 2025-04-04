#include "ParticleCommon.hlsli"

StructuredBuffer<Particle> totalParticlePool : register(t0);
AppendStructuredBuffer<uint> deathParticleSet : register(u0);

[numthreads(64, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint index = DTid.x;
	Particle currentParticle = totalParticlePool[index];
	if (currentParticle.life < 1E-3f)
	{
		deathParticleSet.Append(index);
	}
}
