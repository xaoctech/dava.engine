#pragma once

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{
class DecalComponent;
class DecalRenderObject;
class DecalSystem : public SceneSystem
{
public:
    DecalSystem(Scene* scene);

    void Process(float32 timeElapsed) override;
    void ImmediateEvent(Component* component, uint32 event) override;
    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;

private:
    Vector<DecalRenderObject*> decalObjects;
};
}