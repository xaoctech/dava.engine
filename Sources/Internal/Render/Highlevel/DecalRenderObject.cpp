#include "DecalRenderObject.h"

#include "Render/3D/MeshUtils.h"
#include "Render/Material/NMaterial.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(DecalRenderObject)
{
    ReflectionRegistrator<DecalRenderObject>::Begin()
    .End();
}

DecalRenderObject::DecalRenderObject()
{
    type = eType::TYPE_DECAL;
    bbox = AABBox3(Vector3(0.0f, 0.0f, 0.0f) - size, Vector3(0.0f, 0.0f, 0.0f) + size);
}

void DecalRenderObject::SetDecalSize(const Vector3& _size)
{
    size = _size;
    if (splineRenderData == nullptr)
        bbox = AABBox3(Vector3(0.0f, 0.0f, 0.0f) - size, Vector3(0.0f, 0.0f, 0.0f) + size);
}

DecalRenderObject::~DecalRenderObject()
{
    SafeDelete(splineRenderData);
}

DecalRenderObject::SplineRenderData::~SplineRenderData()
{
    rhi::DeleteVertexBuffer(vertexBuffer);
    rhi::DeleteIndexBuffer(indexBuffer);
}

void DecalRenderObject::SetSplineData(DecalRenderObject::SplineRenderData* splineData)
{
    SafeDelete(splineRenderData);
    splineRenderData = splineData;
    if (splineRenderData)
        bbox = AABBox3(Vector3(splineRenderData->resBox.min.x, splineRenderData->resBox.min.y, 0.0f), Vector3(splineRenderData->resBox.max.x, splineRenderData->resBox.max.y, 0.0f));
    else
        bbox = AABBox3(Vector3(0.0f, 0.0f, 0.0f) - size, Vector3(0.0f, 0.0f, 0.0f) + size);
}

void DecalRenderObject::SplineDataUpdated()
{
    bbox = AABBox3(Vector3(splineRenderData->resBox.min.x, splineRenderData->resBox.min.y, 0.0f), Vector3(splineRenderData->resBox.max.x, splineRenderData->resBox.max.y, 0.0f));
}

DecalRenderObject::SplineRenderData* DecalRenderObject::GetSplineData() const
{
    return splineRenderData;
}

void DecalRenderObject::RecalcBoundingBox()
{
}

void DecalRenderObject::RecalculateWorldBoundingBox()
{
    //GFX_COMPLETE - this will recalc full 3d transform even for vt decals - so just dont rotate vt decals for now in 3d
    RenderObject::RecalculateWorldBoundingBox();

    if (splineRenderData != nullptr)
    {
        //we need to update spline boxes
        for (SplineRenderData::SegmentData& seg : splineRenderData->segmentData)
        {
            Vector2 ex = Vector2(worldTransform->data[0], worldTransform->data[1]);
            Vector2 ey = Vector2(worldTransform->data[4], worldTransform->data[5]);
            Vector2 p = Vector2(worldTransform->data[12], worldTransform->data[13]);
            Vector2 minT = seg.bbox.min.x * ex + seg.bbox.min.y * ey + p;
            Vector2 deltaTX = (seg.bbox.max.x - seg.bbox.min.x) * ex;
            Vector2 deltaTY = (seg.bbox.max.y - seg.bbox.min.y) * ey;
            seg.worldBbox.min = minT;
            seg.worldBbox.max = minT;
            seg.worldBbox.AddPoint(minT + deltaTX);
            seg.worldBbox.AddPoint(minT + deltaTY);
            seg.worldBbox.AddPoint(minT + deltaTX + deltaTY);
        }
    }
}
}