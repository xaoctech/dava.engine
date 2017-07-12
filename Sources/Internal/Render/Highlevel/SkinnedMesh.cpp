#include "Render/Highlevel/SkinnedMesh.h"
#include "Render/Renderer.h"

namespace DAVA
{
SkinnedMesh::SkinnedMesh()
{
    type = TYPE_SKINNED_MESH;
}

RenderObject* SkinnedMesh::Clone(RenderObject* newObject)
{
    if (!newObject)
    {
        DVASSERT(IsPointerToExactClass<SkinnedMesh>(this), "Can clone only SkinnedMesh");
        newObject = new SkinnedMesh();
    }
    RenderObject::Clone(newObject);
    return newObject;
}

void SkinnedMesh::RecalcBoundingBox()
{
    AABBox3 geometryBBox;

    for (const IndexedRenderBatch& i : renderBatchArray)
    {
        RenderBatch* batch = i.renderBatch;
        geometryBBox.AddAABBox(batch->GetBoundingBox());
    }

    float32 radius = geometryBBox.GetBoundingSphereRadius();
    bbox = AABBox3(geometryBBox.GetCenter(), 2.f * radius);
}

void SkinnedMesh::BindDynamicParameters(Camera* camera)
{
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_JOINTS_COUNT, &jointsCount, reinterpret_cast<pointer_size>(&jointsCount));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_JOINT_POSITIONS, &positionArray[0], reinterpret_cast<pointer_size>(positionArray));
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_JOINT_QUATERNIONS, &quaternionArray[0], reinterpret_cast<pointer_size>(quaternionArray));

    RenderObject::BindDynamicParameters(camera);
}
}