#include "DefineLinkedWithCpp.hlsli"

struct Particle
{
    float4 color;
    float3 worldPos;
    float3 velocity;
    float3 accelerate;
    uint emitterType;
    uint emitterID;
    float life;
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