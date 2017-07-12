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

    virtual RenderObject* Clone(RenderObject* newObject);

    void RecalcBoundingBox() override;
    void BindDynamicParameters(Camera* camera) override;

    inline void SetBoundingBox(const AABBox3& box);
    inline void SetJointsPtr(Vector4* positionPtr, Vector4* quaternoinPtr, uint32 count);

protected:
    Vector4* positionArray = nullptr;
    Vector4* quaternionArray = nullptr;
    uint32 jointsCount = 0;
};

inline void SkinnedMesh::SetJointsPtr(Vector4* positionPtr, Vector4* quaternoinPtr, uint32 count)
{
    positionArray = positionPtr;
    quaternionArray = quaternoinPtr;
    jointsCount = count;
}

inline void SkinnedMesh::SetBoundingBox(const AABBox3& box)
{
    bbox = box;
}

} //ns

#endif