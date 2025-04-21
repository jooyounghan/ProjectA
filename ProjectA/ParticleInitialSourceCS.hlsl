#include "SourceCommon.hlsli"

cbuffer EmitterSpawnProperty : register(b3)
{
	matrix transformation;
	float2 minInitRadian;
	float2 maxInitRadian;
	float2 minMaxRadius;
	uint initialParticleCount;
	float initialParticleLife;
	float3 color;
	uint emitterSpawnPropertyDummy;
}

[numthreads(LocalThreadCount, 1, 1)]
void main( uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID )
{
	float groupID = Gid.x;
	uint threadID = DTid.x;

	if (threadID < initialParticleCount)
	{
		uint revivedIndex = deathParticleSet.Consume();

		float dt2 = dt * dt;
        float2 randRads = lerp(minInitRadian, maxInitRadian, hash22(float2(threadID * dt2, groupID * dt2)));
		float randRadius = lerp(minMaxRadius.x, minMaxRadius.y, rand(float2(groupID * dt2, threadID * dt2)));
		float3 randPos = randRadius * float3(cos(randRads.x) * cos(randRads.y), sin(randRads.y), sin(randRads.x) * cos(randRads.y));
		float4 worldPos = mul(mul(float4(randPos, 1.f), transformation), emitterWorldTransformation);
		worldPos.xyz /= worldPos.w;

		Particle sourcedParticle;
		sourcedParticle.worldPos = worldPos.xyz;
		sourcedParticle.velocity = float3(0.f, 0.f, 0.f);
		sourcedParticle.accelerate = float3(0.f, 0.f, 0.f);
		sourcedParticle.emitterID = emitterID;
		sourcedParticle.emitterType = emitterType;
		sourcedParticle.life = initialParticleLife;
		sourcedParticle.color = color;
		sourcedParticle.particleDummy = emitterSpawnPropertyDummy;
		totalParticles[revivedIndex] = sourcedParticle;
		aliveFlags[revivedIndex] = 1;
	}
}