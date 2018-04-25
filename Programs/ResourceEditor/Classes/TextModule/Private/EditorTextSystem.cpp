#include "Classes/TextModule/Private/EditorTextSystem.h"

#include <REPlatform/Scene/SceneEditor2.h>
#include <REPlatform/Scene/Systems/TextDrawSystem.h>

#include <Debug/DVAssert.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/TextComponent.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Utils/Utils.h>
#include <Math/Transform.h>

DAVA_VIRTUAL_REFLECTION_IMPL(EditorTextSystem)
{
    using namespace DAVA;
    ReflectionRegistrator<EditorTextSystem>::Begin()[M::SystemTags("resource_editor", "text_editor")]
    .ConstructorByPointer<Scene*>()
    .End();
}

EditorTextSystem::EditorTextSystem(DAVA::Scene* scene)
    : SceneSystem(scene, DAVA::ComponentUtils::MakeMask<DAVA::TextComponent>())
{
}

void EditorTextSystem::AddEntity(DAVA::Entity* entity)
{
    DVASSERT(entity->GetComponentCount<DAVA::TextComponent>() > 0);
    textEntities.push_back(entity);
}

void EditorTextSystem::RemoveEntity(DAVA::Entity* entity)
{
    bool removed = DAVA::FindAndRemoveExchangingWithLast(textEntities, entity);
    DVASSERT(removed);
}

void EditorTextSystem::PrepareForRemove()
{
    textEntities.clear();
}

void EditorTextSystem::Draw()
{
    using namespace DAVA;

    if (IsSystemEnabled() == false)
        return;

    SceneEditor2* sc = static_cast<SceneEditor2*>(GetScene());
    TextDrawSystem* drawSystem = sc->GetSystem<TextDrawSystem>();

    for (Entity* entity : textEntities)
    {
        uint32 count = entity->GetComponentCount<DAVA::TextComponent>();

        float32 textHeight = 0.f;
        for (uint32 i = 0; i < count; ++i)
        {
            TextComponent* tc = entity->GetComponent<DAVA::TextComponent>(i);
            if (tc->IsVisible() && tc->GetText().empty() == false)
            {
                textHeight += tc->GetSize();
            }
        }

        TransformComponent* transform = entity->GetComponent<TransformComponent>();
        Vector3 position = transform->GetWorldTransform().GetTranslation();
        Vector2 position2D = drawSystem->ToPos2d(position);
        position2D.y += textHeight / 2.f;

        float32 offset = 0.f;
        for (uint32 i = 0; i < count; ++i)
        {
            TextComponent* tc = entity->GetComponent<DAVA::TextComponent>(i);
            if (tc->IsVisible() && tc->GetText().empty() == false)
            {
                offset += tc->GetSize();
                drawSystem->DrawText(position2D - Vector2(0.f, offset), tc->GetText(), tc->GetColor(), tc->GetSize(), TextDrawSystem::Align::TopCenter);
            }
        }
    }
}
