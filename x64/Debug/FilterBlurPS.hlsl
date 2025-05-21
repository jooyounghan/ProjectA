#include "FilterCommon.hlsli"

cbuffer BlurFilmProperty : register(b2)
{
    float blurRadius;
    uint3 blurFilterDummy;
};

Texture2D filterSource : register(t0);
SamplerState clampSampler : register(s0);

static float2 offsets[9] = {
    float2(-1.f, -1.f), float2(0.f, -1.f), float2(1.f, -1.f),
    float2(-1.f,  0.f), float2(0.f,  0.f), float2(1.f,  0.f),
    float2(-1.f,  1.f), float2(0.f,  1.f), float2(1.f,  1.f)
};

static float weights[9] = {
    1.f/16.f, 2.f/16.f, 1.f/16.f,
    2.f/16.f, 4.f/16.f, 2.f/16.f,
    1.f/16.f, 2.f/16.f, 1.f/16.f
};

float4 main(PostProcessVertexOutput input) : SV_TARGET
{
    uint width, height, numLevels;
    filterSource.GetDimensions(0, width, height, numLevels);
    
    float4 color = float4(0, 0, 0, 0);
    float2 scale = float2(1.f / width, 1.f / height);

    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        color += filterSource.Sample(
            clampSampler, 
            input.f2TexCoord + blurRadius * offsets[i] * scale
        ) * weights[i];
    }
   
    return color;
}