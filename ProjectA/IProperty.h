#pragma once
#include "Updatable.h"

class IProperty : public IUpdatable
{
public:
    virtual ~IProperty() = default;

public:
    virtual void DrawPropertyUI() = 0;
};

class APropertyOnEmitterTimeline : public IProperty
{
public:
    APropertyOnEmitterTimeline(float& emitterCurrentTime, float& emitterLoopTime);
    ~APropertyOnEmitterTimeline() override = default;

protected:
    float& m_emitterCurrentTime;
    float& m_emitterLoopTime;

public:
    virtual void AdjustControlPointsFromLoopTime() = 0;
};