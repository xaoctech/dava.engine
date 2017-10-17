#include "Modules/CreatingControlsModule/CreatingControlsData.h"

DAVA_VIRTUAL_REFLECTION_IMPL(CreatingControlsData)
{
    DAVA::ReflectionRegistrator<CreatingControlsData>::Begin()
    .ConstructorByPointer()
    .End();
}