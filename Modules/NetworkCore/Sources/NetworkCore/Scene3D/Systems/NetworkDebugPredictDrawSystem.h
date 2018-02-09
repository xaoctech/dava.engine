#pragma once

#include <Entity/SceneSystem.h>
#include <Base/BaseTypes.h>
#include <Math/Matrix4.h>

namespace DAVA
{
class Scene;
class Entity;
class NetworkTransformComponent;
class Color;
class NetworkDebugPredictDrawSystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(NetworkDebugPredictDrawSystem, SceneSystem);

    NetworkDebugPredictDrawSystem(Scene* scene);

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;

    void Process(DAVA::float32 timeElapsed) override;
    void PrepareForRemove() override{};

private:
    UnorderedMap<Entity*, Matrix4> entityToLastTransform;
    std::unique_ptr<NetworkTransformComponent> tmpNetTransComp;
    const uint32 netTransCompId;

    void DrawBox(const Entity* entity, const Matrix4& transform, const Color& color);
};

} //namespace DAVA
