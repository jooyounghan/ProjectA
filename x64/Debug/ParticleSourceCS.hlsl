#include "SourceCommon.hlsli"

cbuffer EmitterManagerProperties : register(b2)
{
    uint particleMaxCount;
    uint emitterType;
    uint2 emitterPropertyDummy;
};

cbuffer EmitterProperties : register(b3)
{
    uint emitterID;
    uint3 emitterManagerPropertyDummy;
};

#ifdef RUNTIME_SOURCE
cbuffer SpawnProperty : register(b4)
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
	float4 color;
}
#elif defined(INITIAL_SOURCE)
cbuffer SpawnProperty : register(b4)
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
	float4 color;
}
#else
cbuffer SpawnProperty : register(b4)
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
	float4 color;
}
#endif

StructuredBuffer<matrix> emitterWorldTransforms : register(t0);

RWStructuredBuffer<Particle> totalParticles : register(u0);
ConsumeStructuredBuffer<uint> deathIndexSet : register(u1);
AppendStructuredBuffer<uint> aliveIndexSet : register(u2);

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

	uint revivedIndex = deathIndexSet.Consume();

	Particle sourcedParticle;

	const float pMaxCount = float(particleMaxCount);
	const float rand1 = pMaxCount;
	const float rand2 = pMaxCount + 1;
	const float rand3 = pMaxCount + 2;
	const float rand4 = pMaxCount + 3;
	const matrix emitterWorldTransform = emitterWorldTransforms[emitterID];

    float2 randPositionRads = lerp(minPositionRadian, maxPositionRadian, hash22(float2(revivedIndex / rand1, revivedIndex / rand1)));
	float randPositionRadius = lerp(minMaxRadius.x, minMaxRadius.y, rand(float2(revivedIndex / rand2, revivedIndex / rand2)));
	float3 randPos = randPositionRadius * float3(cos(randPositionRads.x) * cos(randPositionRads.y), sin(randPositionRads.y), sin(randPositionRads.x) * cos(randPositionRads.y));
	float4 worldPos = mul(mul(float4(randPos, 1.f), positionTransformation), emitterWorldTransform);
	worldPos.xyz /= worldPos.w;

	float2 randSpeedRads = lerp(minSpeedRadian, maxSpeedRadian, hash22(float2(revivedIndex / rand3, revivedIndex / rand3)));
	float speedHash = rand(float2(revivedIndex / rand4, revivedIndex / rand4));
	float randSpeed = lerp(minMaxSpeed.x, minMaxSpeed.y, speedHash);
	float3 velocity = randSpeed * float3(cos(randSpeedRads.x) * cos(randSpeedRads.y), sin(randSpeedRads.y), sin(randSpeedRads.x) * cos(randSpeedRads.y));
	float4 worldVelocity = mul(mul(float4(velocity, 0.f), speedTransformation), emitterWorldTransform);
	
	sourcedParticle.worldPos = worldPos.xyz;
	sourcedParticle.velocity = worldVelocity.xyz;
	sourcedParticle.accelerate = float3(0.f, 0.f, 0.f);
	sourcedParticle.color = color;
	sourcedParticle.emitterID = emitterID;
	sourcedParticle.xyScale = float2(1.f, 1.f);
	sourcedParticle.dummy = uint3(0, 0, 0);

#ifdef RUNTIME_SOURCE
	sourcedParticle.life = life;
#elif defined(INITIAL_SOURCE)
	sourcedParticle.life = initialParticleLife;
#else
	sourcedParticle.life = 0.f;
#endif
	totalParticles[revivedIndex] = sourcedParticle;
    aliveIndexSet.Append(revivedIndex);
}