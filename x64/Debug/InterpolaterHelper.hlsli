#include "DefineLinkedWithCpp.hlsli"

#define MaxStepCount MaxControlPointsCount - 1

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
