struct ParticleVSOut
{
    float4 viewPos : POSITION;
    float2 velocity : VELOCITY;
    float4 color : COLOR;
    float2 xyScale : SCALE;
};

struct SpriteVSOut
{
    float4 viewPos : POSITION;
    float4 color : COLOR;
    float2 xyScale : SCALE;
    float spriteIndex : SPRITEINDEX;
    uint emitterID : EMITTERID;
};

struct ParticleGSOut
{
    float4 viewPos : SV_Position;
    float2 velocity : VELOCITY;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
};

struct SpriteGSOut
{
    float4 viewPos : SV_Position;
    float2 texCoord : TEXCOORD;
    float4 color : COLOR;
    float spriteIndex : SPRITEINDEX;
    uint emitterID : EMITTERID;
};

struct ParticlePSOut
{
    float4 f4ShotFilm : SV_Target0;
    float4 f4BlurFilm : SV_Target1;
    float2 f2MotionVector : SV_Target2;
};

struct SpritePSOut
{
    float4 f4ShotFilm : SV_Target0;
    float4 f4BlurFilm : SV_Target1;
};

float smoothstep(float edge0, float edge1, float x)
{
    x = clamp((x - edge0) / (edge1 - edge0), 0, 1);
    return x * x * (3.f - 2.f * x);
}