#pragma once

#include "Base/BaseTypes.h"
#include "Entity/SceneSystem.h"

namespace DAVA
{
class Component;
class SkeletonComponent;
class Mesh;
class RenderSystem;
class RenderHelper;
class Entity;

class SkeletonSystem : public SceneSystem
{
public:
    DAVA_VIRTUAL_REFLECTION(SkeletonSystem, SceneSystem);

    SkeletonSystem(Scene* scene);
    ~SkeletonSystem();

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;
    void PrepareForRemove() override;

    void ImmediateEvent(Component* component, uint32 event) override;
    void Process(float32 timeElapsed) override;

    void UpdateMeshJointTransforms(SkeletonComponent* skeleton, Mesh* mesh);
    void UpdateMeshBoundingBox(SkeletonComponent* skeleton, Mesh* mesh);

    void DrawSkeletons(RenderHelper* drawer);

private:
    void UpdateJointTransforms(SkeletonComponent* skeleton);

    void RebuildSkeleton(SkeletonComponent* skeleton);

    void UpdateTestSkeletons(float32 timeElapsed);

    Vector<Entity*> entities;
};

} //ns
