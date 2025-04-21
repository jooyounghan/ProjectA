#include "SourceCommon.hlsli"

cbuffer ParticleSpawnProperty : register(b3)
{
	matrix transformation;
	float2 minSpeedRadian;
	float2 maxSpeedRadian;
	float2 minMaxSpeed;
	float2 minMaxLifeTime;
	float3 color;
	uint particleSpawnPropertyDummy;
}

[numthreads(1, 1, 1)]
void main(uint3 Gid : SV_GroupID, uint3 DTid : SV_DispatchThreadID)
{
	float groupID = Gid.x;
	uint threadID = DTid.x;

	uint revivedIndex = deathParticleSet.Consume();

	float4 worldPos = mul(float4(0.f, 0.f, 0.f, 1.f), emitterWorldTransformation);
	worldPos.xyz /= worldPos.w;

	Particle sourcedParticle;
	sourcedParticle.worldPos = worldPos.xyz;

	float dt2 = dt * dt;
	float2 randRads = lerp(minSpeedRadian, maxSpeedRadian, hash22(float2(revivedIndex * dt, revivedIndex * dt2)));
	float speedHash = rand(float2(revivedIndex * dt2, revivedIndex * dt));
	float randSpeed = lerp(minMaxSpeed.x, minMaxSpeed.y, speedHash);
	float3 velocity = randSpeed * float3(cos(randRads.x) * cos(randRads.y), sin(randRads.y), sin(randRads.x) * cos(randRads.y));
	float4 worldVelocity = mul(mul(float4(velocity, 0.f), transformation), emitterWorldTransformation);
	

	sourcedParticle.velocity = worldVelocity.xyz;
	sourcedParticle.accelerate = float3(0.f, 0.f, 0.f);
	sourcedParticle.emitterID = emitterID;
	sourcedParticle.emitterType = emitterType;
	sourcedParticle.life = lerp(minMaxLifeTime.x, minMaxLifeTime.y, rand(float2(dt2, dt2)));
	sourcedParticle.color = color;
	sourcedParticle.particleDummy = particleSpawnPropertyDummy;

	totalParticles[revivedIndex] = sourcedParticle;
    aliveFlags[revivedIndex] = 1;
}