#define LocalThreadCount 64

struct Particle
{
    float3 worldPos;
    float life;
    float3 velocity;
    float density;
    float3 accelerate;
    uint type;
    float radius;
    float3 dummy;
};

cbuffer AppParams : register(b0)
{
    float dt;
    float appWidth;
    float appHeight;
    uint particleMaxCount;
};