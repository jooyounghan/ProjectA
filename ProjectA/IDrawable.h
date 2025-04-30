#pragma once
#include "imgui.h"

class IDrawable
{
public:
    virtual ~IDrawable() = default;

public:
    virtual void DrawUI() = 0;

protected:
    virtual void DrawUIImpl() = 0;
};
