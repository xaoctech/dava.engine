#pragma once

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{
class EnvironmentComponent;
class EnvironmentSystem : public SceneSystem
{
public:
    EnvironmentSystem(Scene* scene);

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;

    void Process(float32 timeElapsed) override;

    void PrepareForRemove() override
    {
    }

private:
    void UpdateFog(EnvironmentComponent* component);

private:
    Entity* entityWithEnvironmentComponent = nullptr;
    Vector4 fogValues = Vector4(1.0f, 0.0f, 0.0f, 0.0f);
};
}
