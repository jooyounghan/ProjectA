#include "ParticleDrawCommon.hlsli"

[maxvertexcount(4)]
void main(
    point ParticleVSOut input[1], uint primID : SV_PrimitiveID, 
    inout TriangleStream<ParticleGSOut> output
)
{
    ParticleVSOut inputData = input[0];
    const float2 xyScale = inputData.xyScale;
    const float4 dx = float4(10.f / appWidth, 0.f, 0.f, 0.f) * xyScale.x;
    const float4 dy = float4(0.f, 10.f / appHeight, 0.f, 0.f) * xyScale.y;

    ParticleGSOut element;
    element.color = inputData.color;

    element.viewPos = inputData.viewPos - dx - dy;
    element.texCoord = float2(0.f, 1.f);
    output.Append(element);

    element.viewPos = inputData.viewPos - dx + dy;
    element.texCoord = float2(0.f, 0.f);
    output.Append(element);
    
    element.viewPos = inputData.viewPos + dx - dy;
    element.texCoord = float2(1.f, 1.f);
    output.Append(element);
    
    element.viewPos = inputData.viewPos + dx + dy;
    element.texCoord = float2(1.f, 0.f);
    output.Append(element);
    
    output.RestartStrip();

}