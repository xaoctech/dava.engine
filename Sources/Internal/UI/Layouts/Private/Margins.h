#pragma once

#include "Base/BaseTypes.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
struct Margins : public ReflectionBase
{
    Margins();
    Margins(float32 left, float32 top, float32 right, float32 bottom);

    float32 left = 0.f;
    float32 top = 0.f;
    float32 right = 0.f;
    float32 bottom = 0.f;

private:
    DAVA_VIRTUAL_REFLECTION(Margins, ReflectionBase);
};
}
