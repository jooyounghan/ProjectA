#include "ParticleSimulateCommon.hlsli"

RWStructuredBuffer<Particle> totalParticles : register(u1);
RWStructuredBuffer<uint> aliveFlags : register(u2);
ConsumeStructuredBuffer<uint> deathParticleSet : register(u3);

cbuffer EmitterProperties : register(b1)
{
	matrix toWorldTransformation;
	float3 emitVelocity;
	uint emitterID;
	uint emitterType;
	float particleMass;
	uint emitterDummy[2];
};

[numthreads(1, 1, 1)]
void main( )
{
	uint revivedIndex = deathParticleSet.Consume();

	Particle sourcedParticle;
	float4 worldPos = mul(float4(0.f, 0.f, 0.f, 1.f), toWorldTransformation);
	worldPos.xyz /= worldPos.w;

	sourcedParticle.worldPos = worldPos.xyz;
	sourcedParticle.life = 6.f;

	float3 randomVelocity = float3(rand(float2(1.f / revivedIndex, dt)), rand(float2(2.f / revivedIndex, dt/ 2.f)), rand(float2(3.f / revivedIndex, dt / 3.f)));

	sourcedParticle.velocity = mul(float4(emitVelocity + randomVelocity, 0.f), toWorldTransformation).xyz;
	sourcedParticle.mass = particleMass;
	sourcedParticle.accelerate = float3(0.f, -9.8f, 0.f);
	sourcedParticle.type = emitterType;

	totalParticles[revivedIndex] = sourcedParticle;
    aliveFlags[revivedIndex] = 1;
}