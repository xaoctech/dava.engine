#pragma once

#include <Base/BaseTypes.h>
#include <Base/FastName.h>
#include <Entity/SceneSystem.h>

namespace DAVA
{
class CollisionSingleComponent;
class VisualScriptSystem : public SceneSystem
{
public:
    //    DAVA_VIRTUAL_REFLECTION(VisualScriptSystem, SceneSystem);

    VisualScriptSystem(Scene* scene);

    void Process(float32 timeElapsed) override;
    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;

protected:
    void SetScene(Scene* scene) override;

private:
    CollisionSingleComponent* collisionSingleComponent = nullptr;
    UnorderedSet<Entity*> visualScripts;
    UnorderedMap<FastName, Vector<Entity*>> eventToScripts;
};
}
