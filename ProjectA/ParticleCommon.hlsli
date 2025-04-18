#include "DefineLinkedWithCpp.hlsli"

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
    uint depthInverseBit;
    uint dummy;
};

struct PrefixDesciptor
{
    uint    aggregate;
    uint    statusFlag; /* X : 0, A : 1, P : 2*/
    uint    exclusivePrefix;
    uint    inclusivePrefix;
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