#include "ParticleCommon.hlsli"
#include "FilterCommon.hlsli"

Texture2D filterSource : register(t0);
SamplerState clampSampler : register(s0);

static float2 offsets[4] =
{
    float2(-1.f, -1.f), float2(1.f, -1.f), float2(-1.f, 1.f), float2(1.f, 1.f)
};

float4 main(PostProcessVertexOutput output) : SV_TARGET
{
    uint width, height, numLevels;
    filterSource.GetDimensions(0, width, height, numLevels);
    
    float4 color = float4(0, 0, 0, 0);
    
    [unroll]
    for (int i = 0; i < 4; ++i)
    {
        color += filterSource.Sample(
            clampSampler, 
            output.f2TexCoord + offsets[i] * float2(1.f / width, 1.f / height)
        );
    }
   
    return color * 0.25;
}