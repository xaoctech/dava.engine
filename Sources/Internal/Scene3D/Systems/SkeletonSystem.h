#ifndef __DAVAENGINE_SKELETON_SYSTEM_H__
#define __DAVAENGINE_SKELETON_SYSTEM_H__

#include "Base/BaseTypes.h"
#include "Scene3D/Systems/BaseProcessSystem.h"

namespace DAVA
{
class Component;
class SkeletonComponent;
class SkinnedMesh;
class RenderHelper;

class SkeletonSystem : public SceneSystem
{
public:
    SkeletonSystem(Scene* scene);
    ~SkeletonSystem();

    void AddEntity(Entity* entity) override;
    void RemoveEntity(Entity* entity) override;

    void Process(float32 timeElapsed) override;

    void DrawSkeletons(RenderHelper* drawer);

private:
    void UpdatePose(SkeletonComponent* component);
    void UpdateSkinnedMesh(SkeletonComponent* component, SkinnedMesh* skinnedMeshObject);

    void RebuildSkeleton(Entity* entity);

    Vector<Entity*> entities;
};

} //ns

#endif