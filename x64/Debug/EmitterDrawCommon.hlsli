cbuffer CameraViewProj : register(b0)
{
	matrix viewProjMatrix;
};

struct EmitterVSInput
{
    float3 modelPos : POSITION;
    float3 normalVec : NORMAL;
    float4 world0 : INSTANCE_WORLD0;
    float4 world1 : INSTANCE_WORLD1;
    float4 world2 : INSTANCE_WORLD2;
    float4 world3 : INSTANCE_WORLD3;
};

struct EmitterVSOutput
{
    float4 viewProjPos : SV_POSITION;
    float3 normalVec : NORMAL;
};

struct EmitterPSOutput
{
    float4 color : SV_TARGET0;
    float4 normalVec : SV_TARGET1;
};
