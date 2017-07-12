#include "Animation2/AnimationClip.h"
#include "Animation2/SkeletonAnimation.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/MotionComponent.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Scene3D/Systems/MotionSystem.h"
#include "Debug/ProfilerCPU.h"
#include "Debug/ProfilerMarkerNames.h"

namespace DAVA
{
MotionSystem::MotionSystem(Scene* scene)
    : SceneSystem(scene)
{
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::SKELETON_CONFIG_CHANGED);
    scene->GetEventSystem()->RegisterSystemForEvent(this, EventSystem::MOTION_CHANGED);
}

MotionSystem::~MotionSystem()
{
    GetScene()->GetEventSystem()->UnregisterSystemForEvent(this, EventSystem::SKELETON_CONFIG_CHANGED);
    GetScene()->GetEventSystem()->UnregisterSystemForEvent(this, EventSystem::MOTION_CHANGED);
}

void MotionSystem::AddEntity(Entity* entity)
{
    SkeletonComponent* skeleton = GetSkeletonComponent(entity);
    MotionComponent* motion = GetMotionComponent(entity);

    DVASSERT(std::find_if(animations.begin(), animations.end(), [&skeleton](const std::pair<SkeletonComponent*, SkeletonAnimation*>& element) {
                 return element.first == skeleton;
             }) == animations.end());

    animations.emplace_back(skeleton, new SkeletonAnimation());
    animations.back().second->BindAnimation(motion->animationClip, skeleton);
}

void MotionSystem::RemoveEntity(Entity* entity)
{
    SkeletonComponent* skeleton = GetSkeletonComponent(entity);

    auto found = std::find_if(animations.begin(), animations.end(), [&skeleton](const std::pair<SkeletonComponent*, SkeletonAnimation*>& element) {
        return element.first == skeleton;
    });

    DVASSERT(found != animations.end());

    if (found != animations.end())
    {
        SafeDelete(found->second);

        *found = animations.back();
        animations.pop_back();
    }
}

void MotionSystem::ImmediateEvent(Component* component, uint32 event)
{
    if (event == EventSystem::SKELETON_CONFIG_CHANGED || event == EventSystem::MOTION_CHANGED)
    {
        SkeletonComponent* skeleton = GetSkeletonComponent(component->GetEntity());
        MotionComponent* motion = GetMotionComponent(component->GetEntity());

        auto found = std::find_if(animations.begin(), animations.end(), [&skeleton](const std::pair<SkeletonComponent*, SkeletonAnimation*>& element) {
            return element.first == skeleton;
        });

        if (found != animations.end())
            found->second->BindAnimation(motion->animationClip, skeleton);
    }
}

void MotionSystem::Process(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE(ProfilerCPUMarkerName::SCENE_MOTION_SYSTEM);

    for (auto& anim : animations)
    {
        SkeletonComponent* skeleton = anim.first;
        SkeletonAnimation* animation = anim.second;

        animation->Advance(timeElapsed);
        skeleton->ApplyPose(animation->GetSkeletonPose());
    }
}
}