#include "Classes/Common/EntityTypeCreator.h"

#include <Reflection/ReflectionRegistrator.h>

DAVA_VIRTUAL_REFLECTION_IMPL(EntityTypeCreatorBase)
{
    DAVA::ReflectionRegistrator<EntityTypeCreatorBase>::Begin()
    .End();
}

DAVA_VIRTUAL_REFLECTION_IMPL(EntityTypeCreator)
{
    DAVA::ReflectionRegistrator<EntityTypeCreator>::Begin()
    .End();
}

DAVA_VIRTUAL_REFLECTION_IMPL(EntityMultipleTypesCreator)
{
    DAVA::ReflectionRegistrator<EntityMultipleTypesCreator>::Begin()
    .End();
}