#include "Classes/SignatureModule/Private/OwnersSignatureSystem.h"

#include <REPlatform/Global/StringConstants.h>

#include <FileSystem/KeyedArchive.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/SingleComponents/TransformSingleComponent.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Systems/EventSystem.h>
#include <Time/DateTime.h>
#include <Utils/StringFormat.h>

DAVA_VIRTUAL_REFLECTION_IMPL(OwnersSignatureSystem)
{
    using namespace DAVA;
    ReflectionRegistrator<OwnersSignatureSystem>::Begin()[M::SystemTags("resource_editor", "signature")]
    .ConstructorByPointer<Scene*>()
    .Method("Process", &OwnersSignatureSystem::Process)[M::SystemProcessInfo(SPI::Group::Gameplay, SPI::Type::Normal, 20.0f)]
    .End();
}

OwnersSignatureSystem::OwnersSignatureSystem(DAVA::Scene* scene)
    : SceneSystem(scene, DAVA::ComponentMask())
{
}

void OwnersSignatureSystem::SetUserName(const DAVA::String& userName)
{
    currentUserName = userName;
}

void OwnersSignatureSystem::AddEntity(DAVA::Entity* entity)
{
    UpdateOwner(entity);
}

void OwnersSignatureSystem::Process(DAVA::float32 timeElapsed)
{
    const DAVA::TransformSingleComponent* tsc = GetScene()->GetSingleComponentForRead<DAVA::TransformSingleComponent>(this);
    for (DAVA::Entity* entity : tsc->localTransformChanged)
    {
        UpdateOwner(entity);
    }
    for (DAVA::Entity* entity : tsc->transformParentChanged)
    {
        UpdateOwner(entity);
    }
}

void OwnersSignatureSystem::UpdateOwner(DAVA::Entity* entity)
{
    if (IsSystemEnabled() == false)
    {
        return;
    }

    DAVA::KeyedArchive* properties = DAVA::GetCustomPropertiesArchieve(entity);
    if (nullptr != properties)
    {
        properties->SetString(DAVA::ResourceEditor::SCENE_NODE_DESIGNER_NAME_PROPERTY_NAME, currentUserName);

        DAVA::DateTime now = DAVA::DateTime::Now();
        DAVA::String timeString = DAVA::Format("%04d.%02d.%02d_%02d_%02d_%02d",
                                               now.GetYear(), now.GetMonth() + 1, now.GetDay(),
                                               now.GetHour(), now.GetMinute(), now.GetSecond());

        properties->SetString(DAVA::ResourceEditor::SCENE_NODE_MODIFICATION_DATA_PROPERTY_NAME, timeString);
    }
}
