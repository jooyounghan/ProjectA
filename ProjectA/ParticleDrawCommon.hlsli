#include "ParticleCommon.hlsli"

struct ParticleVSOut
{
    float4 viewPos : POSITION0;
    float3 color : COLOR;
    float life : LIFE;
};

struct ParticleGSOut
{
    float4 viewPos : SV_Position;
    float2 texCoord : TEXCOORD;
    float3 color : COLOR;
    float life : LIFE;
};

float smoothstep(float edge0, float edge1, float x)
{
    x = clamp((x - edge0) / (edge1 - edge0), 0, 1);
    return x * x * (3.f - 2.f * x);
}

StructuredBuffer<Particle> particles : register(t0);
StructuredBuffer<uint> currentIndices : register(t1);