#define LocalThreadCount 64

struct Particle
{
    float3 worldPos;
    float3 velocity;
    float3 accelerate;
    float life;
    float density;
    uint emitterID;
    uint emitterType;
    float radius;
    float2 dummy;
};

struct ParticleSelector
{
    float4 viewPos;
    uint index;
    uint emitterType;
    float depth;
    float dummy;
};


cbuffer AppParams : register(b0)
{
    float dt;
    float appWidth;
    float appHeight;
    uint particleMaxCount;
};

cbuffer CameraViewProj : register(b1)
{
    matrix viewProjMatrix;
    matrix invTransposeViewMatrix;
};