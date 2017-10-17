#include "Modules/LibraryModule/LibraryData.h"

DAVA_VIRTUAL_REFLECTION_IMPL(LibraryData)
{
    DAVA::ReflectionRegistrator<LibraryData>::Begin()
    .ConstructorByPointer()
    .End();
}

const DAVA::Vector<DAVA::RefPtr<ControlNode>>& LibraryData::GetDefaultControls() const
{
    return defaultControls;
}
