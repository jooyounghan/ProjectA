#include "ParticleCommon.hlsli"

ConsumeStructuredBuffer<uint> deathParticleSet : register(u0);
AppendStructuredBuffer<uint> aliveParticleSet : register(u1);
RWStructuredBuffer<Particle> totalParticlePool : register(u2);

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
void main( uint3 DTid : SV_DispatchThreadID )
{
	uint revivedIndex = deathParticleSet.Consume();

	Particle sourcedParticle;
	float4 worldPos = mul(float4(0.f, 0.f, 0.f, 1.f), toWorldTransformation);
	worldPos.xyz /= worldPos.w;

	sourcedParticle.worldPos = worldPos.xyz;
	sourcedParticle.life = 10.f;
	sourcedParticle.velocity = mul(float4(emitVelocity, 0.f), toWorldTransformation).xyz;
	sourcedParticle.mass = particleMass;
	sourcedParticle.accelerate = float3(0.f, -9.8f, 0.f);
	sourcedParticle.type = emitterType;

	totalParticlePool[revivedIndex] = sourcedParticle;
	aliveParticleSet.Append(revivedIndex);
}