#ifndef __DAVAENGINE_ANIMATION_SYSTEM_H__
#define __DAVAENGINE_ANIMATION_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"
#include "Entity/SceneSystem.h"
#include "Math/MathConstants.h"
#include "Math/Matrix4.h"

namespace DAVA
{
class Entity;
class AnimationComponent;

class AnimationSystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(AnimationSystem, SceneSystem);

    AnimationSystem(Scene* scene);
    ~AnimationSystem();

    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;

    void Process(float32 timeElapsed) override;

    void ImmediateEvent(Component* component, uint32 event) override;

private:
    Vector<AnimationComponent*> activeComponents;
    void AddToActive(AnimationComponent* comp);
    void RemoveFromActive(AnimationComponent* comp);
    void MoveAnimationToFrame(AnimationComponent* comp, int frameIndex);
};
};

#endif //__DAVAENGINE_ANIMATION_SYSTEM_H__
