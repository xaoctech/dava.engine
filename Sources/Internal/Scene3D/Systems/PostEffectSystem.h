#pragma once

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"
#include "Math/Vector.h"

namespace DAVA
{
class PostEffectComponent;
class PostEffectRenderer;
class PostEffectSystem : public SceneSystem
{
public:
    PostEffectSystem(Scene* scene);

    void ImmediateEvent(Component* component, uint32 event) override;
    void AddEntity(Entity* entity) override;
    void Process(float timeElapsed) override;

private:
    Vector2 currentDynamicRange;
    float currentTargetLuminance = 0.0f;
};
};
