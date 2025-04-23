#include "DefineLinkedWithCpp.hlsli"

struct Particle
{
    float4 color;
    float3 worldPos;
    float3 velocity;
    float3 accelerate;
    uint emitterType;
    uint emitterID;
    float maxLife;
    float life;
    uint colorInterpolaterID;
    uint colorInterpolaterDegree;
    float particleDummy;
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