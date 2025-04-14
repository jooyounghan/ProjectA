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
            
            // 가속도를 통한 적분
            currentParticle.velocity += currentParticle.accelerate * dt;
            currentParticle.worldPos += currentParticle.velocity * dt;

            if (currentParticle.worldPos.y < 0.f)
            {
                currentParticle.worldPos.y = 1E-3;
                currentParticle.velocity.x = currentParticle.velocity.x * 0.2f;
                currentParticle.velocity.y = currentParticle.velocity.y * -0.2f;
                currentParticle.velocity.z = currentParticle.velocity.z * 0.2f;
            }

            
            totalParticles[index] = currentParticle;
        }		
    }
}
