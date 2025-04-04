#include "ParticleCommon.hlsli"

StructuredBuffer<Particle> totalParticlePool : register(t0);
AppendStructuredBuffer<uint> deathParticleSet : register(u0);
AppendStructuredBuffer<uint> aliveParticleSet : register(u1);

[numthreads(64, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint index = DTid.x;
	Particle currentParticle = totalParticlePool[index];
	if (currentParticle.life < 1E-3f)
	{
		deathParticleSet.Append(index);
	}
	else
	{
		aliveParticleSet.Append(index);
	}
}
