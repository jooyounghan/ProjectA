#include "FilterCommon.hlsli"

Texture2D filterSource : register(t0);
SamplerState clampSampler : register(s0);

float4 main(PostProcessVertexOutput output) : SV_TARGET
{   
    return pow(filterSource.Sample(clampSampler, output.f2TexCoord), 2.2f);

}