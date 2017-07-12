#pragma once

#include "Base/BaseTypes.h"
#include "Scene3D/Systems/BaseProcessSystem.h"

namespace DAVA
{
class AnimationTrack;
class Component;
class Entity;
class SkeletonComponent;

class SkeletonAnimationSystem : public SceneSystem
{
public:
    SkeletonAnimationSystem(Scene* scene);
    ~SkeletonAnimationSystem();

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;

    void ImmediateEvent(Component* component, uint32 event);
    void Process(float32 timeElapsed) override;

private:
    void BindAnimation(SkeletonAnimationComponent* animation, SkeletonComponent* skeleton);

    Vector<Entity*> entities;
};

} //ns
