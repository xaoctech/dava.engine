#include "Scene3D/Systems/TransformSystem.h"

#include "Debug/DVAssert.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Components/AnimationComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/TransformInterpolationComponent.h"
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
    GetEngineContext()->settings->settingChanged.Connect(this, &TransformSystem::OnEngineSettingsChanged);
    OnEngineSettingsChanged(EngineSettings::SETTING_INTERPOLATION_MODE);
}

TransformSystem::~TransformSystem()
{
    GetEngineContext()->settings->settingChanged.Disconnect(this);
}

void TransformSystem::Process(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::SCENE_TRANSFORM_SYSTEM);
    this->timeElapsed = timeElapsed;

    TransformSingleComponent* tsc = GetScene()->GetSingletonComponent<TransformSingleComponent>();
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
    bool updateDone = true;

    if (entity->GetFlags() & Entity::TRANSFORM_NEED_UPDATE || forceUpdate)
    {
        // it entity transform changed we need to force
        // update for all its children in hierarchy
        forceUpdate = true;

        TransformComponent* tc = entity->GetComponent<TransformComponent>();
        if (nullptr != tc->parentMatrix)
        {
            bool isFinal = true;

            Matrix4 transform = GetWorldTransform(tc, &isFinal);
            const Matrix4& parentTransform = *tc->parentMatrix;

            // Finish updating only when calculated world transform is final
            updateDone = isFinal;

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
    DVASSERT(interpolationMode >= EngineSettings::INTERPOLATION_OFF && interpolationMode <= EngineSettings::INTERPOLATION_CUBIC);

    if (EngineSettings::INTERPOLATION_OFF != interpolationMode)
    {
        TransformInterpolationComponent* tic = tc->GetEntity()->GetComponent<TransformInterpolationComponent>();
        if (nullptr != tic && !tic->done)
        {
            float32 fx = 1.0f;

            if (tic->immediately)
            {
                tic->immediately = false;
                tic->done = true;
            }
            else if (tic->time > 0 && tic->elapsed < tic->time)
            {
                float32 lenPosition = (tc->position - tic->curPosition).Length();
                float32 lenScale = (tc->scale - tic->curScale).Length();
                float32 lenRotation = (Vector3(tc->rotation.x, tc->rotation.y, tc->rotation.z) - Vector3(tic->curRotation.x, tic->curRotation.y, tic->curRotation.z)).Length();
                float32 force = std::max({ lenPosition, lenScale, lenRotation }) * tic->spring;

                tic->elapsed += (timeElapsed * (1.0f + force) * interpolationSpeed);
                float32 x = std::min(1.0f, tic->elapsed / tic->time);

                if (interpolationMode == EngineSettings::INTERPOLATION_LINEAR)
                {
                    fx = x;
                }
                else if (interpolationMode == EngineSettings::INTERPOLATION_SIN)
                {
                    static const float32 pi = static_cast<float32>(std::acos(-1));
                    static const float32 pi05 = pi * 0.5f;
                    fx = std::sin(pi05 * x);
                }
                else if (interpolationMode == EngineSettings::INTERPOLATION_LOG)
                {
                    fx = std::max(0.1f, 0.5f * std::log(x) + 1);
                }
                else if (interpolationMode == EngineSettings::INTERPOLATION_CUBIC)
                {
                    fx = x * x * x;
                }
            }
            else
            {
                tic->done = true;
            }

            tic->curPosition = Lerp(tic->startPosition, tc->position, fx);
            tic->curScale = Lerp(tic->startScale, tc->scale, fx);
            tic->curRotation.Slerp(tic->startRotation, tc->rotation, fx);

            *isFinal = tic->done;
            return Matrix4(tic->curPosition, tic->curRotation, tic->curScale);
        }
    }

    *isFinal = true;
    return Matrix4(tc->position, tc->rotation, tc->scale);

    /*
    if (nullptr != tic && interpolationType > 0)
    {
        if (tic->immidiately)
        {
            tic->immidiately = false;
            tic->ResetTime();

            finished = true;
        }
        else if (tic->time > 0 && tic->elapsed < tic->time)
        {
            float s = 1.0f;

            tic->elapsed += timeElapsed;
            if (interpolationType == 1)
            {
                // see on-line plot to understand following formula
                // when speed = 0.5f
                // https://www.desmos.com/calculator/zer40yomns
                //
                float32 x = 9 * tic->elapsed / tic->time;
                s = tic->speed - std::log10(1 + x) * tic->speed;
            }
            else
            {
                float32 x = tic->elapsed / tic->time;
                s = 0.2f + std::sin(x * 3.14f) * 0.2f;
            }

            tc->intermediateTransform.position = Lerp(tc->intermediateTransform.position, tc->localTransform.position, s);
            tc->intermediateTransform.scale = Lerp(tc->intermediateTransform.scale, tc->localTransform.scale, s);
            tc->intermediateTransform.rotation.Slerp(tc->intermediateTransform.rotation, tc->localTransform.rotation, s);
        }
        else
        {
            tic->elapsed = 0;
            finished = true;
        }
    }
    else
    {
        finished = true;
    }

    if (finished)
    {
        tc->position = tc->localTransform.position;
        tc->scale = tc->localTransform.scale;
        tc->rotation = tc->localTransform.rotation;
    }

    interpolationFinished = finished;
    return Matrix4(tc->intermediateTransform.position, tc->intermediateTransform.rotation, tc->intermediateTransform.scale);
    */
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
        { //topmost parent
            DVASSERT(entity->GetRetainCount() >= 1);
            updatableEntities.push_back(entity);
        }
    }
}

void TransformSystem::OnEngineSettingsChanged(EngineSettings::eSetting)
{
    interpolationMode = GetEngineContext()->settings->GetSetting<EngineSettings::SETTING_INTERPOLATION_MODE>().Get<EngineSettings::eSettingValue>();
    interpolationSpeed = GetEngineContext()->settings->GetSetting<EngineSettings::SETTING_INTERPOLATION_SPEED>().Get<float32>();
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

    GetScene()->GetSingletonComponent<TransformSingleComponent>()->EraseEntity(entity);
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
