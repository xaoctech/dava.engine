#ifndef __DAVAENGINE_SKINNED_MESH_H__
#define __DAVAENGINE_SKINNED_MESH_H__

#include "Animation/AnimatedObject.h"
#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/HashMap.h"
#include "Debug/DVAssert.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Highlevel/RenderObject.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{
class PolygonGroup;
class RenderBatch;
class ShadowVolume;
class NMaterial;
struct JointTransform;
class SkinnedMesh : public RenderObject
{
public:
    const static uint32 MAX_TARGET_JOINTS = 32; //same as in shader

    using JointTargets = Vector<int32>; // Vector index is joint target, value - skeleton joint index.

    SkinnedMesh();

    RenderObject* Clone(RenderObject* newObject) override;
    void Save(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Load(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void RecalcBoundingBox() override;
    void BindDynamicParameters(Camera* camera, RenderBatch* batch) override;
    void PrepareToRender(Camera* camera) override;

    void SetBoundingBox(const AABBox3& box);
    void SetFinalJointTransformsPtr(const JointTransform* transformsPtr, uint32 jointCount);

    void SetJointTargets(RenderBatch* batch, const JointTargets& jointTargets);
    JointTargets GetJointTargets(RenderBatch* batch);

protected:
    struct JointTargetsData
    {
        Vector<Vector4> positions;
        Vector<Vector4> quaternions;
        uint32 jointsDataCount = 0;
    };
    HashMap<RenderBatch*, JointTargets> jointTargets;
    HashMap<RenderBatch*, JointTargetsData> jointTargetsData;

    const JointTransform* skeletonFinalJointTransforms = nullptr;
    uint32 skeletonJointCount = 0;
};

inline void SkinnedMesh::SetFinalJointTransformsPtr(const JointTransform* transformsPtr, uint32 jointCount)
{
    skeletonFinalJointTransforms = transformsPtr;
    skeletonJointCount = jointCount;
}

inline void SkinnedMesh::SetJointTargets(RenderBatch* batch, const JointTargets& targets)
{
    DVASSERT(uint32(targets.size()) <= MAX_TARGET_JOINTS);

    jointTargets[batch] = targets;

    JointTargetsData& data = jointTargetsData[batch];
    data.positions.resize(targets.size());
    data.quaternions.resize(targets.size());
    data.jointsDataCount = uint32(targets.size());
}

inline SkinnedMesh::JointTargets SkinnedMesh::GetJointTargets(RenderBatch* batch)
{
    if (jointTargets.count(batch))
        return jointTargets[batch];
    else
        return JointTargets();
}

inline void SkinnedMesh::SetBoundingBox(const AABBox3& box)
{
    bbox = box;
}

} //ns

#endif