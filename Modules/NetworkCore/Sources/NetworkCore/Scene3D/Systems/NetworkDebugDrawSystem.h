#pragma once

#include <Base/BaseTypes.h>
#include <Entity/SceneSystem.h>
#include <Math/Color.h>
#include <Math/Matrix4.h>
#include <Math/Quaternion.h>

namespace DAVA
{
class Scene;
class Entity;
class NetworkTransformComponent;
class Color;
class NetworkDebugDrawSystem : public SceneSystem
{
    static const bool SHOW_ERROR_ONLY = false;
    static const uint32 TTL_TRANSFORM_IN_FRAMES = 32;
    const uint32 MAX_LOST_FRAMES = 1;
    const Color LATE_COLOR = Color(1.0f, 1.0f, 0.0f, 0.5f);
    const Color REAL_COLOR = Color(0.0f, 1.0f, 1.0f, 0.1f);

public:
    DAVA_VIRTUAL_REFLECTION(NetworkDebugDrawSystem, SceneSystem);

    NetworkDebugDrawSystem(Scene* scene);

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void RegisterComponent(Entity* entity, Component* component) override;

    void Process(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override{};

private:
    struct Transform
    {
        Vector3 position;
        Quaternion orientation;
        uint8 numberOfLost = 0;
        uint32 ttl = TTL_TRANSFORM_IN_FRAMES;
    };

    using FrameToTransform = Map<uint32, Transform>;
    UnorderedMap<Entity*, FrameToTransform> entityToTransforms;
};

} //namespace DAVA
