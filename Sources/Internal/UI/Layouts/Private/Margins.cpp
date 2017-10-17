#include "UI/Layouts/Private/Margins.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(Margins)
{
    ReflectionRegistrator<Margins>::Begin()
    .ConstructorByValue()
    .ConstructorByPointer()
    .DestructorByPointer([](Margins* m) { SafeDelete(m); })
    .Field("left", &Margins::left)
    .Field("top", &Margins::top)
    .Field("right", &Margins::right)
    .Field("bottom", &Margins::bottom)
    .End();
}

Margins::Margins() = default;

Margins::Margins(float32 left, float32 top, float32 right, float32 bottom)
    : left(left)
    , top(top)
    , right(right)
    , bottom(bottom)
{
}
}