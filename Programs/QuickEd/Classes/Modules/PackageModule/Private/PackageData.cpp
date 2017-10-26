#include "Modules/PackageModule/PackageData.h"
#include "Modules/PackageModule/PackageWidget.h"

DAVA_VIRTUAL_REFLECTION_IMPL(PackageData)
{
    DAVA::ReflectionRegistrator<PackageData>::Begin()
    .ConstructorByPointer()
    .End();
}
