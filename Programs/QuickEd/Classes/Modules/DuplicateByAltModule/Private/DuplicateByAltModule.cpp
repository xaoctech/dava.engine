#include "Classes/Modules/DuplicateByAltModule/DuplicateByAltModule.h"
#include "Classes/Modules/DuplicateByAltModule/Private/DuplicateByAltModuleData.h"
#include "Classes/Modules/DuplicateByAltModule/Private/DuplicateByAltSystem.h"

#include "Classes/Application/QEGlobal.h"

#include <TArc/Utils/ModuleCollection.h>

DAVA_VIRTUAL_REFLECTION_IMPL(DuplicateByAltModule)
{
    DAVA::ReflectionRegistrator<DuplicateByAltModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void DuplicateByAltModule::PostInit()
{
    std::unique_ptr<DuplicateByAltModuleData> data = std::make_unique<DuplicateByAltModuleData>();
    data->system = std::make_unique<DuplicateByAltSystem>(GetAccessor());
    data->system->duplicateRequest.Connect(this, &DuplicateByAltModule::OnDuplicateRequested);
    GetAccessor()->GetGlobalContext()->CreateData(std::move(data));
}

void DuplicateByAltModule::CreateSystems(Interfaces::EditorSystemsManagerInterface* systemsManager)
{
    DuplicateByAltModuleData* data = GetAccessor()->GetGlobalContext()->GetData<DuplicateByAltModuleData>();
    DVASSERT(data != nullptr);
    DuplicateByAltSystem* system = data->system.get();

    systemsManager->RegisterEditorSystem(system);
}

void DuplicateByAltModule::DestroySystems(Interfaces::EditorSystemsManagerInterface* systemsManager)
{
    DuplicateByAltModuleData* data = GetAccessor()->GetGlobalContext()->GetData<DuplicateByAltModuleData>();
    DVASSERT(data != nullptr);
    DuplicateByAltSystem* system = data->system.get();

    systemsManager->UnregisterEditorSystem(system);
}

void DuplicateByAltModule::OnDuplicateRequested()
{
    InvokeOperation(QEGlobal::Duplicate.ID);
}

DECL_GUI_MODULE(DuplicateByAltModule);
