#ifndef __DAVAENGINE_SKINNED_MESH_H__
#define __DAVAENGINE_SKINNED_MESH_H__

#include "Base/BaseTypes.h"
#include "Animation/AnimatedObject.h"
#include "Base/BaseMath.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Highlevel/RenderObject.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{
class PolygonGroup;
class RenderBatch;
class ShadowVolume;
class NMaterial;

class SkinnedMesh : public RenderObject
{
public:
    const static uint32 MAX_TARGET_JOINTS = 32; //same as in shader

    SkinnedMesh();

    RenderObject* Clone(RenderObject* newObject) override;
    void Save(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Load(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void RecalcBoundingBox() override;
    void BindDynamicParameters(Camera* camera, RenderBatch* batch) override;
    void PrepareToRender(Camera* camera) override;

    void SetBoundingBox(const AABBox3& box);
    void SetJointsPtr(const Vector4* positionPtr, const Vector4* quaternoinPtr, uint32 jointCount);
    void SetJointsMapping(RenderBatch* batch, const Vector<int32>& jointTargets);

protected:
    struct BatchJointData
    {
        Vector<Vector4> positions;
        Vector<Vector4> quaternions;
        uint32 jointsDataCount = 0;
    };
    Map<RenderBatch*, Vector<int32>> jointsMapping; // [batch, [target joints]]
    Map<RenderBatch*, BatchJointData> jointsData; // [batch, [target joints]]

    const Vector4* positionArray = nullptr;
    const Vector4* quaternionArray = nullptr;
    uint32 skeletonJointCount = 0;
};

inline void SkinnedMesh::SetJointsPtr(const Vector4* positionPtr, const Vector4* quaternoinPtr, uint32 jointCount)
{
    positionArray = positionPtr;
    quaternionArray = quaternoinPtr;
    skeletonJointCount = jointCount;
}

inline void SkinnedMesh::SetJointsMapping(RenderBatch* batch, const Vector<int32>& jointTargets)
{
    jointsMapping[batch] = jointTargets;
    jointsData[batch].positions.resize(jointTargets.size());
    jointsData[batch].quaternions.resize(jointTargets.size());
    jointsData[batch].jointsDataCount = uint32(jointTargets.size());
}

inline void SkinnedMesh::SetBoundingBox(const AABBox3& box)
{
    bbox = box;
}

} //ns

#endif