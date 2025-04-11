#include "SourceCommon.hlsli"

cbuffer EmitterSpawnProperty : register(b2)
{
	float2 minInitRadians;
	float2 maxInitRadians;
	float2 minMaxInitRadius;
	uint initialParticleCount;
	uint emitterSpawnPropertyDummy;
}

[numthreads(LocalThreadCount, 1, 1)]
void main( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID )
{
	float groupID = Gid.x;
	uint index = DTid.x;
	if (index < initialParticleCount)
	{
		uint revivedIndex = deathParticleSet.Consume();

		float dt2 = dt * dt;
		float2 randRads = lerp(minInitRadians, maxInitRadians, hash22(float2(index * dt2, index * dt2)));
		float randRadius = lerp(minMaxInitRadius.x, minMaxInitRadius.y, rand(float2(groupID * dt2, index * dt2)));
		float3 randPos = randRadius * float3(cos(randRads.x) * cos(randRads.y), sin(randRads.y), sin(randRads.x) * cos(randRads.y));
		float4 worldPos = mul(float4(randPos, 1.f), toWorldTransformation);
		worldPos.xyz /= worldPos.w;

		Particle sourcedParticle;
		sourcedParticle.worldPos = worldPos.xyz;
		sourcedParticle.life = 10.f;
		sourcedParticle.velocity = float3(0.f, 0.f, 0.f);
		sourcedParticle.density = particleDensity;
		sourcedParticle.accelerate = float3(0.f, -9.8 * (particleDensity - 1.f), 0.f);
		sourcedParticle.type = emitterType;


		totalParticles[revivedIndex] = sourcedParticle;
		aliveFlags[revivedIndex] = 1;
	}
}