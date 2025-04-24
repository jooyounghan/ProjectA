#include "DefineLinkedWithCpp.hlsli"

#define MaxStepCount MaxControlPointsCount - 1

struct EmitterInterpolaterInformation
{
    float maxLife;
    uint colorInterpolaterID;
    uint colorInterpolaterDegree;
    float emitterInterpolaterInformationDummy;
};

struct InterpolaterHeader
{
    uint controlPointsCount;
    uint interpolaterFlag;
};

struct Degree1Dim4InterpolaterProperty
{
    InterpolaterHeader header;
    float xProfiles[MaxControlPointsCount];
    float coefficient[MaxStepCount][4][2];
};

struct Degree3Dim4InterpolaterProperty
{
    InterpolaterHeader header;
    float xProfiles[MaxControlPointsCount];
    float coefficient[MaxStepCount][4][4];
};

StructuredBuffer<EmitterInterpolaterInformation> emitterInterpolaterInformations : register(t0);
StructuredBuffer<Degree1Dim4InterpolaterProperty> firstDegreeInterpolaters : register(t1);
StructuredBuffer<Degree3Dim4InterpolaterProperty> thirdDegreeInterpolaters : register(t2);

static float Get3DegreeFunction(float a, float b, float c, float d, float x) { return (((a * x) + b) * x + c) * x + d; }

static float4 Evaluate1Degree(float time, float coeff[4][2])
{
    return float4(
        coeff[0][0] * time + coeff[0][1],
        coeff[1][0] * time + coeff[1][1],
        coeff[2][0] * time + coeff[2][1],
        coeff[3][0] * time + coeff[3][1]
        );
}

static float4 Evaluate3Degree(float t, float coeff[4][4])
{
    return float4(
        Get3DegreeFunction(coeff[0][0], coeff[0][1], coeff[0][2], coeff[0][3], t),
        Get3DegreeFunction(coeff[1][0], coeff[1][1], coeff[1][2], coeff[1][3], t),
        Get3DegreeFunction(coeff[2][0], coeff[2][1], coeff[2][2], coeff[2][3], t),
        Get3DegreeFunction(coeff[3][0], coeff[3][1], coeff[3][2], coeff[3][3], t)
        );
}


float4 GetInterpolated(uint degree, uint interpolatedID, float4 timeSpent4, float maxLife)
{
    const uint CubicSplineMethod = 2;
    const uint CatmullRomMethod = 3;
    const float timeSpent = timeSpent4.x;

    if (degree == 2)
    {
        Degree1Dim4InterpolaterProperty interpolaterProperty = firstDegreeInterpolaters[interpolatedID];
        const uint stepsCount = interpolaterProperty.header.controlPointsCount - 1;

        [unroll]
        for (uint stepIdx = 0; stepIdx < stepsCount; ++stepIdx)
        {
            float x1 = interpolaterProperty.xProfiles[stepIdx];
            float x2 = interpolaterProperty.xProfiles[stepIdx + 1];

            if (x1 <= timeSpent && timeSpent < x2)
            {
                return Evaluate1Degree(timeSpent, interpolaterProperty.coefficient[stepIdx]);
            }
        }

        return Evaluate1Degree(maxLife, interpolaterProperty.coefficient[stepsCount]);
    }

    else if (degree == 4)
    {
        Degree3Dim4InterpolaterProperty interpolaterProperty = thirdDegreeInterpolaters[interpolatedID];
        const uint interpolateMethod = interpolaterProperty.header.interpolaterFlag;
        const uint stepsCount = interpolaterProperty.header.controlPointsCount - 1;

        [unroll]
        for (uint stepIdx = 0; stepIdx < stepsCount; ++stepIdx)
        {
            float x1 = interpolaterProperty.xProfiles[stepIdx];
            float x2 = interpolaterProperty.xProfiles[stepIdx + 1];

            if (x1 <= timeSpent && timeSpent < x2)
            {
                float t = (interpolateMethod == CubicSplineMethod)
                    ? timeSpent - x1
                    : (timeSpent - x1) / (x2 - x1);

                return Evaluate3Degree(t, interpolaterProperty.coefficient[stepIdx]);
            }
        }

        return Evaluate3Degree(0.f, interpolaterProperty.coefficient[stepsCount]);
    }

    return float4(0.f, 0.f, 0.f, 1.f);
}