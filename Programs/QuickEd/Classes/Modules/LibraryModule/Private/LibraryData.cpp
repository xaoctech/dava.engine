#include "Modules/LibraryModule/LibraryData.h"

DAVA_VIRTUAL_REFLECTION_IMPL(LibraryData)
{
    DAVA::ReflectionRegistrator<LibraryData>::Begin()
    .ConstructorByPointer()
    .End();
}
