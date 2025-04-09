struct Particle
{
    float3 worldPos;
    float life;
    float3 velocity;
    float mass;
    float3 accelerate;
    uint type;
};

cbuffer AppParams : register(b0)
{
    float dt;
    float appWidth;
    float appHeight;
    uint particleMaxCount;
};