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
DAVA_VIRTUAL_REFLECTION_IMPL(TransformSystem)
{
    ReflectionRegistrator<TransformSystem>::Begin()[M::Tags("base")]
    .ConstructorByPointer<Scene*>()
    .Method("Process", &TransformSystem::Process)[M::SystemProcess(SP::Group::ENGINE_END, SP::Type::NORMAL, 2.0f)]
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
        if (nullptr != tc->parentMatrix)
        {
            Matrix4 transform = GetWorldTransform(tc, &updateDone);
            const Matrix4& parentTransform = *tc->parentMatrix;

            AnimationComponent* animation = entity->GetComponent<AnimationComponent>();
            if (nullptr != animation)
            {
                tc->SetWorldTransform(animation->animationTransform * transform * parentTransform);
            }
            else
            {
                tc->SetWorldTransform(transform * parentTransform);
            }
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

Matrix4 TransformSystem::GetWorldTransform(TransformComponent* tc, bool* isFinal) const
{
    DVASSERT(nullptr != isFinal);

    TransformInterpolatedComponent* tic = tc->GetEntity()->GetComponent<TransformInterpolatedComponent>();
    if (tic)
    {
        *isFinal = false;
        return Matrix4(tic->translation, tic->rotation, tc->GetScale());
    }

    *isFinal = true;
    return Matrix4(tc->position, tc->rotation, tc->scale);
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
