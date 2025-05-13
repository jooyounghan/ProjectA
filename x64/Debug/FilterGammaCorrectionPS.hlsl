#include "FilterCommon.hlsli"

Texture2D filterSource : register(t0);
SamplerState clampSampler : register(s0);

float4 main(PostProcessVertexOutput input) : SV_TARGET
{   
    return pow(filterSource.Sample(clampSampler, input.f2TexCoord), 1.0f / 2.2f);
}