#pragma once
#include "Updatable.h"
#include <memory>

template <typename T>
class IProperty : public IUpdatable
{
public:
    virtual ~IProperty() = default;
    virtual void DrawPropertyUI() = 0;
};