#include "Classes/Modules/HUDModule/HUDModule.h"
#include "Classes/Modules/HUDModule/Private/HUDModuleData.h"
#include "Classes/Modules/HUDModule/Private/HUDSystem.h"

#include "Interfaces/EditorSystemsManagerInteface.h"

#include <TArc/Utils/ModuleCollection.h>

DAVA_VIRTUAL_REFLECTION_IMPL(HUDModule)
{
    DAVA::ReflectionRegistrator<HUDModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void HUDModule::PostInit()
{
    std::unique_ptr<HUDModuleData> data = std::make_unique<HUDModuleData>();
    data->hudSystem = std::make_unique<HUDSystem>(GetAccessor());
    GetAccessor()->GetGlobalContext()->CreateData(std::move(data));
}

void HUDModule::OnInterfaceRegistered(const DAVA::Type* interfaceType)
{
    if (interfaceType == DAVA::Type::Instance<Interfaces::EditorSystemsManagerInterface>())
    {
        HUDModuleData* data = GetAccessor()->GetGlobalContext()->GetData<HUDModuleData>();
        DVASSERT(data != nullptr);
        HUDSystem* system = data->hudSystem.get();

        Interfaces::EditorSystemsManagerInterface* systemsManager = QueryInterface<Interfaces::EditorSystemsManagerInterface>();
        systemsManager->RegisterEditorSystem(system);
    }
}

void HUDModule::OnBeforeInterfaceUnregistered(const DAVA::Type* interfaceType)
{
    if (interfaceType == DAVA::Type::Instance<Interfaces::EditorSystemsManagerInterface>())
    {
        HUDModuleData* data = GetAccessor()->GetGlobalContext()->GetData<HUDModuleData>();
        DVASSERT(data != nullptr);
        HUDSystem* system = data->hudSystem.get();

        Interfaces::EditorSystemsManagerInterface* systemsManager = QueryInterface<Interfaces::EditorSystemsManagerInterface>();
        systemsManager->UnregisterEditorSystem(system);
    }
}

DECL_GUI_MODULE(HUDModule);
