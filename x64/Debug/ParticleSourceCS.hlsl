#include "SourceCommon.hlsli"

#ifdef RUNTIME_SOURCE
cbuffer SpawnProperty : register(b3)
{
	matrix positionTransformation;
	float2 minPositionRadian;
	float2 maxPositionRadian;
	float2 minMaxRadius;

	float life;
	float particleSpawnPropertyDummy1;

	matrix speedTransformation;
	float2 minSpeedRadian;
	float2 maxSpeedRadian;
	float2 minMaxSpeed;
	float2 particleSpawnPropertyDummy2;
}
#elif defined(INITIAL_SOURCE)
cbuffer SpawnProperty : register(b3)
{
	matrix positionTransformation;
	float2 minPositionRadian;
	float2 maxPositionRadian;
	float2 minMaxRadius;

	uint initialParticleCount;
	float initialParticleLife;

	matrix speedTransformation;
	float2 minSpeedRadian;
	float2 maxSpeedRadian;
	float2 minMaxSpeed;
	float2 particleSpawnPropertyDummy2;
}
#else
cbuffer SpawnProperty : register(b3)
{
	matrix positionTransformation;
	float2 minPositionRadian;
	float2 maxPositionRadian;
	float2 minMaxRadius;

	float2 particleSpawnPropertyDummy1;

	matrix speedTransformation;
	float2 minSpeedRadian;
	float2 maxSpeedRadian;
	float2 minMaxSpeed;
	float2 particleSpawnPropertyDummy2;
}
#endif

#ifdef RUNTIME_SOURCE
[numthreads(1, 1, 1)]
#else
[numthreads(LocalThreadCount, 1, 1)]
#endif
void main(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID)
{
	float groupID = Gid.x;
	uint threadID = DTid.x;

#ifdef INITIAL_SOURCE
	if (threadID >= initialParticleCount) return;
#endif

	uint revivedIndex = deathParticleSet.Consume();

	Particle sourcedParticle;

	float dt2 = dt * dt;
    float2 randPositionRads = lerp(minPositionRadian, maxPositionRadian, hash22(float2(revivedIndex * dt2, revivedIndex * dt)));
	float randPositionRadius = lerp(minMaxRadius.x, minMaxRadius.y, rand(float2(revivedIndex * dt, revivedIndex * dt2)));
	float3 randPos = randPositionRadius * float3(cos(randPositionRads.x) * cos(randPositionRads.y), sin(randPositionRads.y), sin(randPositionRads.x) * cos(randPositionRads.y));
	float4 worldPos = mul(mul(float4(randPos, 1.f), positionTransformation), emitterWorldTransformation);
	worldPos.xyz /= worldPos.w;

	float2 randSpeedRads = lerp(minSpeedRadian, maxSpeedRadian, hash22(float2(revivedIndex * dt, revivedIndex * dt2)));
	float speedHash = rand(float2(revivedIndex * dt2, revivedIndex * dt));
	float randSpeed = lerp(minMaxSpeed.x, minMaxSpeed.y, speedHash);
	float3 velocity = randSpeed * float3(cos(randSpeedRads.x) * cos(randSpeedRads.y), sin(randSpeedRads.y), sin(randSpeedRads.x) * cos(randSpeedRads.y));
	float4 worldVelocity = mul(mul(float4(velocity, 0.f), speedTransformation), emitterWorldTransformation);
	
    sourcedParticle.color = float4(1.f, 1.f, 1.f, 1.f);
	sourcedParticle.worldPos = worldPos.xyz;
	sourcedParticle.velocity = worldVelocity.xyz;
	sourcedParticle.accelerate = float3(0.f, 0.f, 0.f);
	sourcedParticle.emitterID = emitterID;
	sourcedParticle.emitterType = emitterType;

#ifdef RUNTIME_SOURCE
	sourcedParticle.life = life;
#elif defined(INITIAL_SOURCE)
	sourcedParticle.life = initialParticleLife;
#else
	sourcedParticle.life = 0.f;
#endif
	totalParticles[revivedIndex] = sourcedParticle;
    aliveFlags[revivedIndex] = 1;
}