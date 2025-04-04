#include "ParticleCommon.hlsli"

// Test
cbuffer SystemParams : register(b0)
{
	float dt;
	float appWidth;
	float appHeight;
	float dummy;
};

RWStructuredBuffer<Particle> totalParticlePool : register(u0);
AppendStructuredBuffer<uint> deathParticleSet : register(u1);

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
		currentParticle.worldPos += currentParticle.velocity * dt;
		currentParticle.velocity += currentParticle.accelerate * dt;
		currentParticle.life -= dt;
		totalParticlePool[index] = currentParticle;
	}
}
