#pragma once
#include "Updatable.h"
#include <memory>

class IProperty : public IUpdatable
{
public:
    virtual ~IProperty() = default;

public:
    virtual void DrawPropertyUI() = 0;
};