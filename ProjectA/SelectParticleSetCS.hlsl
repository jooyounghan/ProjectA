#include "ParticleSimulateCommon.hlsli"

RWStructuredBuffer<Particle> totalParticles : register(u1);
RWStructuredBuffer<uint> aliveFlags : register(u2);
AppendStructuredBuffer<uint> deathParticleSet : register(u3);

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
        }
		else
		{
            currentParticle.velocity += currentParticle.accelerate * dt;
            currentParticle.worldPos += currentParticle.velocity * dt;
            aliveFlags[index] = 1;
            totalParticles[index] = currentParticle;
        }		
    }
}
