#include "FilterCommon.hlsli"

Texture2D traceTexture : register(t0);
SamplerState clampSampler : register(s0);

float4 main(PostProcessVertexOutput input) : SV_TARGET
{   
    return traceTexture.Sample(clampSampler, input.f2TexCoord);
}