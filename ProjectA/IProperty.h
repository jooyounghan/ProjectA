#pragma once
#include "IUpdatable.h"

class IProperty : public IUpdatable
{
public:
    virtual ~IProperty() = default;

public:
    virtual void DrawPropertyUI() = 0;

protected:
    virtual void DrawPropertyUIImpl() = 0;
};
