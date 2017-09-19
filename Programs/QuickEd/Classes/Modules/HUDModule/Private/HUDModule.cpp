#include "Classes/Modules/HUDModule/HUDModule.h"
#include "Classes/Modules/HUDModule/HUDModuleData.h"
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
    data->hudSystem->highlightChanged.Connect(this, &HUDModule::OnHighlightChanged);
    GetAccessor()->GetGlobalContext()->CreateData(std::move(data));
}

void HUDModule::CreateSystems(Interfaces::EditorSystemsManagerInterface* systemsManager)
{
    HUDModuleData* data = GetAccessor()->GetGlobalContext()->GetData<HUDModuleData>();
    DVASSERT(data != nullptr);
    HUDSystem* system = data->hudSystem.get();

    systemsManager->RegisterEditorSystem(system);
}

void HUDModule::DestroySystems(Interfaces::EditorSystemsManagerInterface* systemsManager)
{
    HUDModuleData* data = GetAccessor()->GetGlobalContext()->GetData<HUDModuleData>();
    DVASSERT(data != nullptr);
    HUDSystem* system = data->hudSystem.get();

    systemsManager->UnregisterEditorSystem(system);
}

void HUDModule::OnHighlightChanged(ControlNode* node)
{
    HUDModuleData* data = GetAccessor()->GetGlobalContext()->GetData<HUDModuleData>();
    DVASSERT(data != nullptr);
    data->highlightedNode = node;
}

DECL_GUI_MODULE(HUDModule);
