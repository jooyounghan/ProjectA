#include "ParticleDrawCommon.hlsli"



[maxvertexcount(4)]
void main(
	point ParticleVSOut input[1], uint primID : SV_PrimitiveID,
	inout TriangleStream<ParticleGSOut> output
)
{
    const float4 dx = float4(20.f / appWidth, 0.f, 0.f, 0.f);
    const float4 dy = float4(0.f, 20.f / appHeight, 0.f, 0.f);

    ParticleGSOut element;
    element.life = input[0].life;
    element.color = input[0].color;

    element.viewPos = input[0].viewPos - dx - dy;
    element.texCoord = float2(0.f, 1.f);
    output.Append(element);

    element.viewPos = input[0].viewPos - dx + dy;
    element.texCoord = float2(0.f, 0.f);
    output.Append(element);
    
    element.viewPos = input[0].viewPos + dx - dy;
    element.texCoord = float2(1.f, 1.f);
    output.Append(element);
    
    element.viewPos = input[0].viewPos + dx + dy;
    element.texCoord = float2(1.f, 0.f);
    output.Append(element);
    
    output.RestartStrip();

}