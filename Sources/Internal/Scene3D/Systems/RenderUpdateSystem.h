#ifndef __DAVAENGINE_SCENE3D_RENDERUPDATESYSTEM_H__
#define __DAVAENGINE_SCENE3D_RENDERUPDATESYSTEM_H__

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"
#include "Base/UnordererSet.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
class RenderPass;
class RenderLayer;
class RenderObject;
class RenderBatch;
class Entity;
class Camera;
class RenderSystem;

/**
    \brief This system is required to transfer all changes from scene to render system and update render object properties.
 */
class RenderUpdateSystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(RenderUpdateSystem, SceneSystem);

    RenderUpdateSystem(Scene* scene);

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;

    void RegisterComponent(Entity* entity, Component* component) override;
    void UnregisterComponent(Entity* entity, Component* component) override;

    void PrepareForRemove() override;
    void Process(float32 timeElapsed) override;

private:
    void RegisterRenderObject(Entity* entity, RenderObject* object);
    void UnregisterRenderObject(Entity* entity, RenderObject* object);
    void UpdateRenderObject(Entity* entity, RenderObject* object);
    void UpdateActiveIndexes(Entity* entity, RenderObject* object);

    RenderSystem* renderSystem = nullptr;
    UnorderedSet<RenderObject*> renderObjects;
};

} // ns

#endif /* __DAVAENGINE_RENDER_RENDERSYSTEM_H__ */
