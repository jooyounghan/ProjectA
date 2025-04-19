#pragma once
#include "Updatable.h"

class IProperty : public IUpdatable
{
public:
    virtual ~IProperty() = default;

public:
    virtual void DrawPropertyUI() = 0;
};

class APropertyHasLoopTime : public IProperty
{
public:
    APropertyHasLoopTime(float& loopTime);
    ~APropertyHasLoopTime() override = default;

protected:
    float& m_loopTime;

public:
    virtual void AdjustControlPointsFromLoopTime() = 0;
};