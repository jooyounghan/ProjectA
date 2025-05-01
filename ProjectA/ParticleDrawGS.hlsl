#include "ParticleCommon.hlsli"
#include "ParticleDrawCommon.hlsli"

[maxvertexcount(4)]
#ifdef SPRITE_EMITTER
void main(point SpriteVSOut input[1], uint primID : SV_PrimitiveID, inout TriangleStream<SpriteGSOut> output)
#else
void main(point ParticleVSOut input[1], uint primID : SV_PrimitiveID, inout TriangleStream<ParticleGSOut> output)
#endif
{
#ifdef SPRITE_EMITTER
    SpriteVSOut inputData = input[0];
    SpriteGSOut element;
    element.spriteIndex = inputData.spriteIndex;
	element.emitterID = inputData.emitterID;
#else
    ParticleVSOut inputData = input[0];
    ParticleGSOut element;
#endif

    const float2 xyScale = inputData.xyScale;
    const float4 dx = float4(10.f / appWidth, 0.f, 0.f, 0.f) * xyScale.x;
    const float4 dy = float4(0.f, 10.f / appHeight, 0.f, 0.f) * xyScale.y;

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