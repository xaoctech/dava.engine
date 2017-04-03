#include "Classes/SlotSupportModule/SlotSupportModule.h"
#include "Classes/SlotSupportModule/Private/EditorSlotSystem.h"
#include "Classes/SlotSupportModule/Private/EntityForSlotLoader.h"

#include "Classes/Interfaces/PropertyPanelInterface.h"
#include "Classes/SceneManager/SceneData.h"

#include "Classes/Qt/Scene/SceneEditor2.h"

#include <Entity/Component.h>
#include <Reflection/ReflectionRegistrator.h>

namespace SlotSupportModuleDetails
{
class SlotSupportData : public DAVA::TArc::DataNode
{
public:
    EditorSlotSystem* system = nullptr;
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
    slotSupportData->system = new EditorSlotSystem(scene.Get());

    scene->AddSystem(slotSupportData->system, MAKE_COMPONENT_MASK(Component::SLOT_COMPONENT), Scene::SCENE_SYSTEM_REQUIRE_PROCESS);
}

void SlotSupportModule::OnContextDeleted(DAVA::TArc::DataContext* context)
{
}

void SlotSupportModule::OnInterfaceRegistered(const Type* interfaceType)
{
    if (interfaceType == Type::Instance<Interfaces::PropertyPanelInterface>())
    {
        Interfaces::PropertyPanelInterface* propertyPanel = QueryInterface<Interfaces::PropertyPanelInterface>();
    }
}

void SlotSupportModule::OnBeforeInterfaceUnregistered(const Type* interfaceType)
{
    if (interfaceType == Type::Instance<Interfaces::PropertyPanelInterface>())
    {
        Interfaces::PropertyPanelInterface* propertyPanel = QueryInterface<Interfaces::PropertyPanelInterface>();
    }
}

void SlotSupportModule::PostInit()
{
}
