#include "ParticleCommon.hlsli"

RWStructuredBuffer<Particle> totalParticles : register(u0);
RWStructuredBuffer<uint> aliveFlags : register(u1);
AppendStructuredBuffer<uint> deathParticleSet : register(u2);

[numthreads(LocalThreadCount, 1, 1)]
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint index = DTid.x;
	if (index < particleMaxCount)
    {
		Particle currentParticle = totalParticles[index];
		
        currentParticle.life -= dt;
		if (currentParticle.life < 1E-3f)
		{
            deathParticleSet.Append(index);
            aliveFlags[index] = 0;
        }
		else
		{
            aliveFlags[index] = 1;
            totalParticles[index] = currentParticle;
        }		
    }
}
