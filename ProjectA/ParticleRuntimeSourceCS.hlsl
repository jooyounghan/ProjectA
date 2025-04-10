#include "SourceCommon.hlsli"

cbuffer ParticleSpawnProperty : register(b2)
{
	float2 minEmitRadians;
	float2 maxEmitRadians;
	float emitSpeed;
	float3 particleSpawnPropertyDummy;
}

[numthreads(1, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID)
{
	float groupID = Gid.x;
	uint index = DTid.x;

	uint revivedIndex = deathParticleSet.Consume();

	Particle sourcedParticle;
	float4 worldPos = mul(float4(0.f, 0.f, 0.f, 1.f), toWorldTransformation);
	worldPos.xyz /= worldPos.w;

	sourcedParticle.worldPos = worldPos.xyz;
	sourcedParticle.life = 3.f;

	float2 randRads = lerp(minEmitRadians, maxEmitRadians, hash22(float2(dt * dt, Pcurrent * dt)));
	float3 randomVelocity = emitSpeed * float3(cos(randRads.x)*cos(randRads.y), sin(randRads.y), sin(randRads.x)*cos(randRads.y));

	sourcedParticle.velocity = mul(float4(randomVelocity, 0.f), toWorldTransformation).xyz;
	sourcedParticle.density = particleDensity;
	sourcedParticle.accelerate = float3(0.f, -9.8 * (particleDensity - 1.f), 0.f);
	sourcedParticle.type = emitterType;

	totalParticles[revivedIndex] = sourcedParticle;
    aliveFlags[revivedIndex] = 1;
}