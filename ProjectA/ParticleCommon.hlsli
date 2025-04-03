struct Particle
{
    float3 modelPos;
    float life;
};

cbuffer SystemParams : register(b0)
{
    float dt;
    float appWidth;
    float appHeight;
    float dummy;
};