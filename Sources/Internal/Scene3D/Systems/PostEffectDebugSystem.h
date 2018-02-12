#pragma once

#include "Base/BaseTypes.h"
#include "Math/Vector.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{
class UIEvent;
class PostEffectDebugSystem : public SceneSystem
{
public:
    PostEffectDebugSystem(Scene* scene);

    void Process(float32 timeElapsed) override;
    void ImmediateEvent(Component* component, uint32 event) override;
    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;
    bool Input(UIEvent* uie) override;

private:
    Component* postEffectDebugComponent = nullptr;
    Vector2 lastPointOfInterest;
};
};