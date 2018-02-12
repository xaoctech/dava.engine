#pragma once

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Reflection/Reflection.h"
#include "Render/Highlevel/SkinnedMesh.h"

namespace DAVA
{
class SpeedTreeUpdateSystem;
class SpeedTreeObject : public SkinnedMesh
{
public:
    static const size_t HARMONICS_BUFFER_CAPACITY = 4 * 7; //7 registers (float4)

    SpeedTreeObject();

    void RecalcBoundingBox() override;
    RenderObject* Clone(RenderObject* newObject) override;
    void Save(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Load(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void BindDynamicParameters(Camera* camera, RenderBatch* batch) override;
    void PrepareToRender(Camera* camera) override;

    static PolygonGroup* CreateSortedPolygonGroup(PolygonGroup* pg);

protected:
    static const FastName FLAG_WIND_ANIMATION;
    static const uint32 SORTING_DIRECTION_COUNT = 8;

    static Vector3 GetSortingDirection(uint32 directionIndex);
    static uint32 SelectDirectionIndex(const Vector3& direction);

    AABBox3 CalcBBoxForSpeedTreeGeometry(RenderBatch* rb);
    void UpdateAnimationFlag(int32 maxAnimatedLod);

    void SetInvWorldTransformPtr(const Matrix4* invWT);

private:
    Vector4 wind;
    Vector4 flexibility;
    Vector4 null;

    const Matrix4* invWorldTransform = nullptr;

    DAVA_VIRTUAL_REFLECTION(SpeedTreeObject, RenderObject);
    friend class SpeedTreeUpdateSystem;
};

inline void SpeedTreeObject::SetInvWorldTransformPtr(const Matrix4* invWT)
{
    invWorldTransform = invWT;
}
}
