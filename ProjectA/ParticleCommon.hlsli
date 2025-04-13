#define LocalThreadCount 64

struct Particle
{
    float3 worldPos;
    float life;
    float3 velocity;
    float density;
    float3 accelerate;
    uint emitterID;
    uint emitterType;
    float radius;
    float2 dummy;
};

struct ParticleSelector
{
    uint index;
    uint emitterType;
    float depth;
    uint dummy;
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