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
    SkinnedMesh();

    RenderObject* Clone(RenderObject* newObject) override;
    void BindDynamicParameters(Camera* camera) override;

    void SetObjectSpaceBoundingBox(const AABBox3& box);
    void SetJointsPtr(Vector4* positionPtr, Vector4* quaternoinPtr, int32 count);

    Vector4* GetPositionArray() const;
    Vector4* GetQuaternionArray() const;
    int32 GetJointsArraySize() const;

    void RecalcBoundingBox() override
    {
    }

protected:
    Vector4* positionArray = nullptr;
    Vector4* quaternionArray = nullptr;
    int32 jointsCount = 0;
};

inline void SkinnedMesh::SetJointsPtr(Vector4* positionPtr, Vector4* quaternoinPtr, int32 count)
{
    positionArray = positionPtr;
    quaternionArray = quaternoinPtr;
    jointsCount = count;
}

inline void SkinnedMesh::SetObjectSpaceBoundingBox(const AABBox3& box)
{
    bbox = box;
}

inline Vector4* SkinnedMesh::GetPositionArray() const
{
    return positionArray;
}

inline Vector4* SkinnedMesh::GetQuaternionArray() const
{
    return quaternionArray;
}

inline int32 SkinnedMesh::GetJointsArraySize() const
{
    return jointsCount;
}

} //ns

#endif