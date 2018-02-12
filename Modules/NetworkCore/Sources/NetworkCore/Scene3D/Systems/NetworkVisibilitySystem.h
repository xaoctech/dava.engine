#pragma once

#include <Base/BaseTypes.h>
#include <Base/FastName.h>
#include <Base/UnordererSet.h>
#include <Entity/SceneSystem.h>

namespace DAVA
{
class NetworkVisibilitySingleComponent;
class NetworkVisibilitySystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkVisibilitySystem, SceneSystem);

    NetworkVisibilitySystem(Scene* scene);

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void ProcessFixed(float32 timeElapsed) override;
    void PrepareForRemove() override{};

private:
    UnorderedSet<Entity*> playerEntities;
    NetworkVisibilitySingleComponent* netVisSingleComp;
};

} //namespace DAVA
