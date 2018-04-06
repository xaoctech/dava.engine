#pragma once

#include <Base/BaseTypes.h>
#include <Base/FastName.h>
#include <Entity/SceneSystem.h>

namespace DAVA
{
class CollisionSingleComponent;
class VisualScriptComponent;
class VisualScriptSingleComponent;
class VisualScriptSystem : public SceneSystem
{
public:
    VisualScriptSystem(Scene* scene);

    void Process(float32 timeElapsed) override;
    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;

protected:
    void SetScene(Scene* scene) override;

private:
    CollisionSingleComponent* collisionSingleComponent = nullptr;
    VisualScriptSingleComponent* visualScriptSingleComponent = nullptr;
    UnorderedSet<VisualScriptComponent*> processVisualScripts;
    UnorderedSet<VisualScriptComponent*> collisionVisualScripts;

    void CheckAndInsertScript(VisualScriptComponent* scriptComponent);
};
}
