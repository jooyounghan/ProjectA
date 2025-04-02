cbuffer CameraViewProj : register(b0)
{
    matrix viewProjMatrix;
};

cbuffer EmitterProperties : register(b1)
{
	matrix toWorldTransform;
	float4 emitVelocity;
};

float4 main( float3 modelPos : POSITION ) : SV_POSITION
{
	float4 worldPos = mul(float4(modelPos, 1.f), toWorldTransform);
	return mul(worldPos, viewProjMatrix);
}