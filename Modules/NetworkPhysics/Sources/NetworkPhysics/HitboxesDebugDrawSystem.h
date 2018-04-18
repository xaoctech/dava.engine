#pragma once

#include <Base/BaseTypes.h>
#include <Math/Matrix4.h>
#include <Math/Color.h>
#include <Entity/SceneSystem.h>

namespace DAVA
{
class HitboxesDebugDrawComponent;
class CollisionShapeComponent;
class RenderObject;

/** System that draws hitboxes captured into `HitboxesDebugDrawComponent`. */
class HitboxesDebugDrawSystem final : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(HitboxesDebugDrawSystem, SceneSystem);

    HitboxesDebugDrawSystem(Scene* scene);
    ~HitboxesDebugDrawSystem();

    void ProcessFixed(float32 timeElapsed) override;
    void PrepareForRemove() override{};

private:
    void UpdateRenderObjects(HitboxesDebugDrawComponent& component);
    RenderObject* CreateHitboxRenderObject(const CollisionShapeComponent& shape, Matrix4& worldTransform, Color color, Vector3 sizeDelta);

private:
    ComponentGroup<HitboxesDebugDrawComponent>* hitboxesDebugDrawComponents;

    // For each hitbox, contains render objects representing server and client states
    struct HitboxRenderObjects
    {
        RenderObject* roServer = nullptr;
        Matrix4 roServerWorldTransform;

        RenderObject* roClient = nullptr;
        Matrix4 roClientWorldTransform;
    };

    // Collision shape component -> its render objects
    UnorderedMap<const CollisionShapeComponent*, HitboxRenderObjects> hitboxRenderObjectsMap;

    uint32 renderObjectsVertexLayoutId = UINT32_MAX;
};
} //namespace DAVA
