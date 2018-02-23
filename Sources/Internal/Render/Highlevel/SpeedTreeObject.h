#pragma once

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Reflection/Reflection.h"
#include "Render/Highlevel/Mesh.h"
#include "Render/Highlevel/SkinnedMesh.h"

namespace DAVA
{
class SpeedTreeUpdateSystem;
class SpeedTreeObject : public Mesh
{
public:
    SpeedTreeObject();

    RenderObject* Clone(RenderObject* newObject) override;

    void PrepareToRender(Camera* camera) override;

    void RecalcBoundingBox() override;

    static PolygonGroup* CreateSortedPolygonGroup(PolygonGroup* pg);

    DAVA_DEPRECATED(void Load(KeyedArchive* archive, SerializationContext* serializationContext) override);

protected:
    static const FastName FLAG_WIND_ANIMATION;
    static const uint32 SORTING_DIRECTION_COUNT = 8;

    static Vector3 GetSortingDirection(uint32 directionIndex);
    static uint32 SelectDirectionIndex(const Vector3& direction);

    AABBox3 CalcBBoxForSpeedTreeGeometry(RenderBatch* rb);

    void SetInvWorldTransformPtr(const Matrix4* invWT);

private:
    const Matrix4* invWorldTransform = nullptr;

    friend class SpeedTreeUpdateSystem;
    friend class SpeedTreeComponent;

    DAVA_VIRTUAL_REFLECTION(SpeedTreeObject, RenderObject);
};

inline void SpeedTreeObject::SetInvWorldTransformPtr(const Matrix4* invWT)
{
    invWorldTransform = invWT;
}
}
