#include "Modules/QEClientModule.h"

#include "Interfaces/EditorSystemsManagerInteface.h"

DAVA_VIRTUAL_REFLECTION_IMPL(QEClientModule)
{
    DAVA::ReflectionRegistrator<QEClientModule>::Begin()
    .End();
}

void QEClientModule::OnInterfaceRegistered(const DAVA::Type* interfaceType)
{
    if (interfaceType == DAVA::Type::Instance<Interfaces::EditorSystemsManagerInterface>())
    {
        CreateSystems(QueryInterface<Interfaces::EditorSystemsManagerInterface>());
    }
}

void QEClientModule::OnBeforeInterfaceUnregistered(const DAVA::Type* interfaceType)
{
    if (interfaceType == DAVA::Type::Instance<Interfaces::EditorSystemsManagerInterface>())
    {
        DestroySystems(QueryInterface<Interfaces::EditorSystemsManagerInterface>());
    }
}
