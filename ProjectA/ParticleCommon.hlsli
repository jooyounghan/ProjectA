#include "DefineLinkedWithCpp.hlsli"

struct Particle
{
    float4 color;
    float3 worldPos;
    float3 velocity;
    float3 accelerate;
    uint emitterID;
    float life;
    float2 xyScale;
    float spriteIndex;
    uint2 dummy;
};

struct SpriteAliveIndex
{
    uint index;
    uint depth;
};

struct PrefixSumStatus
{
    uint aggregate;
    uint statusFlag; /* X : 0, A : 1, P : 2*/
    uint exclusivePrefix;
    uint inclusivePrefix;
};

cbuffer AppParams : register(b0)
{
    float dt;
    float appWidth;
    float appHeight;
    uint appDummy;
};

cbuffer CameraViewProj : register(b1)
{
    matrix viewProjMatrix;
    matrix invTransposeViewMatrix;
};