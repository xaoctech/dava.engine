#include "Classes/SlotSupportModule/SlotSupportModule.h"
#include "Classes/SlotSupportModule/Private/EditorSlotSystem.h"
#include "Classes/SlotSupportModule/Private/EntityForSlotLoader.h"

#include "Classes/Interfaces/PropertyPanelInterface.h"
#include "Classes/SlotSupportModule/Private/EntityForSlotLoader.h"
#include "Classes/SceneManager/SceneData.h"

#include "Classes/Qt/Scene/SceneEditor2.h"

#include <TArc/Utils/ModuleCollection.h>

#include <Entity/Component.h>
#include <Reflection/ReflectionRegistrator.h>

namespace SlotSupportModuleDetails
{
class SlotSupportData : public DAVA::TArc::DataNode
{
public:
    std::unique_ptr<EditorSlotSystem> system = nullptr;

    DAVA_VIRTUAL_REFLECTION_IN_PLACE(SlotSupportData, DAVA::TArc::DataNode)
    {
        DAVA::ReflectionRegistrator<SlotSupportData>::Begin()
        .End();
    }
};
}

DAVA_VIRTUAL_REFLECTION_IMPL(SlotSupportModule)
{
    DAVA::ReflectionRegistrator<SlotSupportModule>::Begin()
    .ConstructorByPointer()
    .End();
}

void SlotSupportModule::OnContextCreated(DAVA::TArc::DataContext* context)
{
    using namespace DAVA;

    SceneData* data = context->GetData<SceneData>();
    RefPtr<SceneEditor2> scene = data->GetScene();

    SlotSupportModuleDetails::SlotSupportData* slotSupportData = new SlotSupportModuleDetails::SlotSupportData();
    context->CreateData(std::unique_ptr<DAVA::TArc::DataNode>(slotSupportData));
    slotSupportData->system.reset(new EditorSlotSystem(scene.Get()));

    scene->AddSystem(slotSupportData->system.get(), MAKE_COMPONENT_MASK(Component::SLOT_COMPONENT), Scene::SCENE_SYSTEM_REQUIRE_PROCESS);
    scene->slotSystem->SetExternalEntityLoader(RefPtr<SlotSystem::ExternalEntityLoader>(new EntityForSlotLoader(GetAccessor())));
}

void SlotSupportModule::OnContextDeleted(DAVA::TArc::DataContext* context)
{
    using namespace DAVA;

    {
        SceneData* data = context->GetData<SceneData>();
        RefPtr<SceneEditor2> scene = data->GetScene();

        SlotSupportModuleDetails::SlotSupportData* slotSupportData = context->GetData<SlotSupportModuleDetails::SlotSupportData>();
        scene->slotSystem->SetExternalEntityLoader(RefPtr<SlotSystem::ExternalEntityLoader>());
        scene->RemoveSystem(slotSupportData->system.get());
    }

    context->DeleteData<SlotSupportModuleDetails::SlotSupportData>();
}

void SlotSupportModule::OnInterfaceRegistered(const DAVA::Type* interfaceType)
{
    if (interfaceType == DAVA::Type::Instance<Interfaces::PropertyPanelInterface>())
    {
        Interfaces::PropertyPanelInterface* propertyPanel = QueryInterface<Interfaces::PropertyPanelInterface>();
    }
}

void SlotSupportModule::OnBeforeInterfaceUnregistered(const DAVA::Type* interfaceType)
{
    if (interfaceType == DAVA::Type::Instance<Interfaces::PropertyPanelInterface>())
    {
        Interfaces::PropertyPanelInterface* propertyPanel = QueryInterface<Interfaces::PropertyPanelInterface>();
    }
}

void SlotSupportModule::PostInit()
{
}

DECL_GUI_MODULE(SlotSupportModule);
