#ifndef __DAVAENGINE_SCENE3D_DEBUGRENDERSYSTEM_H__
#define __DAVAENGINE_SCENE3D_DEBUGRENDERSYSTEM_H__

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{
class Camera;
class Component;
class RenderObject;
class RenderSystem;

class DebugRenderSystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(DebugRenderSystem, SceneSystem);

    DebugRenderSystem(Scene* scene);
    ~DebugRenderSystem();

    void Process(float32 timeElapsed) override;
    void RegisterComponent(Entity* entity, Component* component) override;
    void UnregisterComponent(Entity* entity, Component* component) override;
    void PrepareForRemove() override;

private:
    UnorderedMap<Component*, RenderObject*> componentRenderObjectMap;
    RenderSystem* renderSystem = nullptr;
};
}

#endif //__DAVAENGINE_SCENE3D_DEBUGRENDERSYSTEM_H__
