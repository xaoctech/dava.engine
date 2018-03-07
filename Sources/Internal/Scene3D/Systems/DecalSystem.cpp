#include "Scene3D/Systems/DecalSystem.h"

#include "Entity/ComponentUtils.h"
#include "Scene3D/Components/DecalComponent.h"
#include "Render/Highlevel/DecalRenderObject.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/SingleComponents/TransformSingleComponent.h"

namespace DAVA
{
DecalSystem::DecalSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<DecalComponent>())
{
    if (scene)
    {
        scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::DECAL_COMPONENT_CHANGED);
    }
}

void DecalSystem::Process(float32 timeElapsed)
{
    TransformSingleComponent* tsc = GetScene()->GetSingletonComponent<TransformSingleComponent>();
    for (auto& pair : tsc->worldTransformChanged.map)
    {
        if (pair.first->GetComponentsCount(Type::Instance<DecalComponent>()) > 0)
        {
            for (Entity* entity : pair.second)
            {
                DecalComponent* decalComponent = entity->GetComponent<DecalComponent>();

                // Update new transform pointer, and mark that transform is changed
                const Matrix4* worldTransformPointer = GetTransformComponent(entity)->GetWorldTransformPtr();
                const Matrix4* prevWorldTransformPointer = GetTransformComponent(entity)->GetPrevWorldTransformPtr();
                RenderObject* object = decalComponent->renderObject;
                if (NULL != object)
                {
                    object->SetWorldTransformPtr(worldTransformPointer);
                    object->SetPrevWorldTransformPtr(prevWorldTransformPointer);
                    entity->GetScene()->renderSystem->MarkForUpdate(object);
                }
            }
        }
    }
}

void DecalSystem::ImmediateEvent(Component* component, uint32 event)
{
    Entity* entity = component->GetEntity();
    DecalComponent* decalComponent = entity->GetComponent<DecalComponent>();

    if (event == EventSystem::DECAL_COMPONENT_CHANGED)
    {
        decalComponent->renderObject->SetDecalSize(decalComponent->decalSize);
        decalComponent->renderObject->SetMaterial(decalComponent->material.Get());
        decalComponent->renderObject->SetSortingOffset(decalComponent->sortingOffset);
        entity->GetScene()->renderSystem->MarkForUpdate(decalComponent->renderObject);
    }
}
void DecalSystem::AddEntity(Entity* entity)
{
    DecalComponent* component = entity->GetComponent<DecalComponent>();
    if (component)
    {
        component->renderObject = new DecalRenderObject();

        const Matrix4* worldTransformPointer = GetTransformComponent(entity)->GetWorldTransformPtr();
        const Matrix4* prevWorldTransformPointer = GetTransformComponent(entity)->GetPrevWorldTransformPtr();
        component->renderObject->SetWorldTransformPtr(worldTransformPointer);
        component->renderObject->SetPrevWorldTransformPtr(prevWorldTransformPointer);
        component->renderObject->SetMaterial(component->material.Get());
        component->renderObject->SetDecalSize(component->decalSize);
        component->renderObject->SetSortingOffset(component->sortingOffset);
        GetScene()->GetRenderSystem()->RenderPermanent(component->renderObject);
    }
}
void DecalSystem::RemoveEntity(Entity* entity)
{
    DecalComponent* component = entity->GetComponent<DecalComponent>();
    if (component && component->renderObject)
    {
        DecalRenderObject* decalObject = component->renderObject;

        GetScene()->GetRenderSystem()->RemoveFromRender(decalObject);
        FindAndRemoveExchangingWithLast(decalObjects, decalObject);
        SafeRelease(decalObject);
    }
}

void DecalSystem::PrepareForRemove()
{
    for (DecalRenderObject* object : decalObjects)
    {
        GetScene()->GetRenderSystem()->RemoveFromRender(object);
        SafeRelease(object);
    }
    decalObjects.clear();
}
}
