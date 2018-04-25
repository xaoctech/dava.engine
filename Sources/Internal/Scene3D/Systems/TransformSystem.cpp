#include "Scene3D/Systems/TransformSystem.h"

#include "Debug/DVAssert.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Components/AnimationComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/TransformInterpolatedComponent.h"
#include "Scene3D/Components/SingleComponents/TransformSingleComponent.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"

namespace DAVA
{
const float32 TransformSystem::systemOrder = 2.f;

DAVA_VIRTUAL_REFLECTION_IMPL(TransformSystem)
{
    ReflectionRegistrator<TransformSystem>::Begin()[M::SystemTags("base")]
    .ConstructorByPointer<Scene*>()
    .Method("Process", &TransformSystem::Process)[M::SystemProcessInfo(SPI::Group::EngineEnd, SPI::Type::Normal, TransformSystem::systemOrder)]
    .End();
}

TransformSystem::TransformSystem(Scene* scene)
    : SceneSystem(scene, ComponentUtils::MakeMask<TransformComponent>())
{
}

TransformSystem::~TransformSystem()
{
}

void TransformSystem::Process(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::SCENE_TRANSFORM_SYSTEM);
    this->timeElapsed = timeElapsed;

    const TransformSingleComponent* tsc = GetScene()->GetSingleComponentForRead<TransformSingleComponent>(this);
    for (Entity* e : tsc->localTransformChanged)
    {
        EntityNeedUpdate(e);
        HierarchicAddToUpdate(e);
    }
    for (Entity* e : tsc->transformParentChanged)
    {
        EntityNeedUpdate(e);
        HierarchicAddToUpdate(e);
    }
    for (Entity* e : tsc->animationTransformChanged)
    {
        EntityNeedUpdate(e);
        HierarchicAddToUpdate(e);
    }

    passedNodes = 0;
    multipliedNodes = 0;

    uint32 size = static_cast<uint32>(updatableEntities.size());
    for (uint32 i = 0; i < size; ++i)
    {
        if (UpdateEntity(updatableEntities[i]))
        {
            updatableEntities[i] = nullptr;
        }
    }

    auto it = std::remove(updatableEntities.begin(), updatableEntities.end(), nullptr);
    updatableEntities.erase(it, updatableEntities.end());
}

bool TransformSystem::UpdateEntity(Entity* entity, bool forceUpdate /* = false */)
{
    // Finish updating only when calculated world transform is final
    bool updateDone = true;

    if (entity->GetFlags() & Entity::TRANSFORM_NEED_UPDATE || forceUpdate)
    {
        // it entity transform changed we need to force
        // update for all its children in hierarchy
        forceUpdate = true;

        TransformComponent* tc = entity->GetComponent<TransformComponent>();
        if (tc->parentTransform)
        {
            Transform localTransform(tc->localTransform);

            TransformInterpolatedComponent* tic = tc->GetEntity()->GetComponent<TransformInterpolatedComponent>();
            if (tic)
            {
                localTransform.SetTranslation(tic->translation);
                localTransform.SetRotation(tic->rotation);
                updateDone = false;
            }

            AnimationComponent* animation = entity->GetComponent<AnimationComponent>();
            if (nullptr != animation)
            {
                tc->worldTransform = Transform(animation->animationTransform) * localTransform * *(tc->parentTransform);
            }
            else
            {
                tc->worldTransform = localTransform * *(tc->parentTransform);
            }

            tc->MarkWorldChanged();
        }

        // calculated number of recalculated entities transform
        multipliedNodes++;
    }

    uint32 size = entity->GetChildrenCount();
    for (uint32 i = 0; i < size; ++i)
    {
        Entity* child = entity->GetChild(i);
        if (child->GetFlags() & Entity::TRANSFORM_DIRTY || forceUpdate)
        {
            // We can think that update is done for specified entity
            // only when update done for all its children.
            // So we have to merge all returned values with
            // current `updateDone` flag.
            updateDone &= UpdateEntity(child, forceUpdate);
        }
    }

    // reset flags if update was finished
    if (updateDone)
    {
        entity->RemoveFlag(Entity::TRANSFORM_NEED_UPDATE | Entity::TRANSFORM_DIRTY);
    }

    return updateDone;
}

void TransformSystem::EntityNeedUpdate(Entity* entity)
{
    entity->AddFlag(Entity::TRANSFORM_NEED_UPDATE);
}

void TransformSystem::HierarchicAddToUpdate(Entity* entity)
{
    if (!(entity->GetFlags() & Entity::TRANSFORM_DIRTY))
    {
        entity->AddFlag(Entity::TRANSFORM_DIRTY);
        Entity* parent = entity->GetParent();
        if (parent && parent->GetParent())
        {
            HierarchicAddToUpdate(entity->GetParent());
        }
        else
        {
            //topmost parent
            DVASSERT(entity->GetRetainCount() >= 1);
            updatableEntities.push_back(entity);
        }
    }
}

void TransformSystem::AddEntity(Entity* entity)
{
    EntityNeedUpdate(entity);
    HierarchicAddToUpdate(entity);
}

void TransformSystem::RemoveEntity(Entity* entity)
{
    //TODO: use hashmap
    uint32 size = static_cast<uint32>(updatableEntities.size());
    for (uint32 i = 0; i < size; ++i)
    {
        if (updatableEntities[i] == entity)
        {
            updatableEntities[i] = updatableEntities[size - 1];
            updatableEntities.pop_back();
            break;
        }
    }

    entity->RemoveFlag(Entity::TRANSFORM_NEED_UPDATE);
    entity->RemoveFlag(Entity::TRANSFORM_DIRTY);
}

void TransformSystem::UnregisterEntity(Entity* entity)
{
    SceneSystem::UnregisterEntity(entity);

    DVASSERT(GetScene() != nullptr);

    GetScene()->GetSingleComponent<TransformSingleComponent>()->EraseEntity(entity);
}

void TransformSystem::PrepareForRemove()
{
    uint32 size = static_cast<uint32>(updatableEntities.size());
    for (uint32 i = 0; i < size; ++i)
    {
        Entity* entity = updatableEntities[i];
        entity->RemoveFlag(Entity::TRANSFORM_NEED_UPDATE);
        entity->RemoveFlag(Entity::TRANSFORM_DIRTY);
    }

    updatableEntities.clear();
}
};
