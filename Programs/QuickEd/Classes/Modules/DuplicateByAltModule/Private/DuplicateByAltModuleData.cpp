#include "Classes/Modules/DuplicateByAltModule/Private/DuplicateByAltModuleData.h"
#include "Classes/Modules/DuplicateByAltModule/Private/DuplicateByAltSystem.h"

DAVA_VIRTUAL_REFLECTION_IMPL(DuplicateByAltModuleData)
{
    DAVA::ReflectionRegistrator<DuplicateByAltModuleData>::Begin()
    .ConstructorByPointer()
    .End();
}

DuplicateByAltModuleData::~DuplicateByAltModuleData() = default;
