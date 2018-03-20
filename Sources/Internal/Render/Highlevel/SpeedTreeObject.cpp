#include "Render/3D/MeshUtils.h"
#include "Render/Highlevel/SpeedTreeObject.h"
#include "Render/Material/NMaterialNames.h"
#include "Utils/Utils.h"
#include "Render/Renderer.h"
#include "Render/Highlevel/RenderPassNames.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(SpeedTreeObject)
{
    ReflectionRegistrator<SpeedTreeObject>::Begin()
    .End();
}

const FastName SpeedTreeObject::FLAG_WIND_ANIMATION("WIND_ANIMATION");

SpeedTreeObject::SpeedTreeObject()
{
    type = TYPE_SPEED_TREE;
    AddFlag(SkinnedMesh::CUSTOM_PREPARE_TO_RENDER);
}

void SpeedTreeObject::RecalcBoundingBox()
{
    bbox = AABBox3();

    uint32 size = uint32(renderBatchArray.size());
    for (uint32 k = 0; k < size; ++k)
    {
        RenderBatch* rb = renderBatchArray[k].renderBatch;
        bbox.AddAABBox(CalcBBoxForSpeedTreeGeometry(rb));
    }
}

void SpeedTreeObject::BindDynamicParameters(Camera* camera, RenderBatch* batch)
{
    SkinnedMesh::BindDynamicParameters(camera, batch);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_WIND, wind.data, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_PREV_WIND, prevWind.data, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
    Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_FLEXIBILITY, flexibility.data, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
}

void SpeedTreeObject::PrepareToRender(Camera* camera)
{
    SkinnedMesh::PrepareToRender(camera);

    Vector3 direction = GetWorldBoundingBox().GetCenter() - camera->GetPosition();
    direction = MultiplyVectorMat3x3(direction, *invWorldTransform);
    direction.z = 0.f;
    direction.Normalize();
    uint32 directionIndex = SelectDirectionIndex(direction);

    for (RenderBatch* batch : activeRenderBatchArray)
    {
        PolygonGroup* pg = batch->GetPolygonGroup();
        if (pg)
        {
            int32 meshIndexCount = pg->GetPrimitiveCount() * 3;
            if (meshIndexCount != pg->GetIndexCount()) //sorted polygon group
            {
                uint32 startIndex = meshIndexCount * directionIndex;
                DVASSERT(uint32(pg->GetIndexCount()) >= uint32(startIndex + meshIndexCount));

                batch->startIndex = startIndex;
            }
        }
    }
}

void SpeedTreeObject::UpdateAnimationFlag(int32 maxAnimatedLod)
{
    uint32 size = uint32(renderBatchArray.size());
    for (uint32 k = 0; k < size; ++k)
    {
        int32 flagValue = (renderBatchArray[k].lodIndex > maxAnimatedLod) ? 0 : 1;

        auto material = renderBatchArray[k].renderBatch->GetMaterial();
        if (material->HasLocalFlag(FLAG_WIND_ANIMATION))
        {
            material->SetFlag(FLAG_WIND_ANIMATION, flagValue);
        }
        else
        {
            material->AddFlag(FLAG_WIND_ANIMATION, flagValue);
        }
        material->PreCacheFX();
    }
}

RenderObject* SpeedTreeObject::Clone(RenderObject* newObject)
{
    if (!newObject)
    {
        DVASSERT(IsPointerToExactClass<SpeedTreeObject>(this), "Can clone only SpeedTreeObject");
        newObject = new SpeedTreeObject();
    }

    SkinnedMesh::Clone(newObject);
    return newObject;
}

void SpeedTreeObject::Save(KeyedArchive* archive, SerializationContext* serializationContext)
{
    SkinnedMesh::Save(archive, serializationContext);
}

void SpeedTreeObject::Load(KeyedArchive* archive, SerializationContext* serializationContext)
{
    SkinnedMesh::Load(archive, serializationContext);

    //RHI_COMPLETE TODO: Remove setting WIND_ANIMATION flag. We need to add/set flag manually (and save it) to reduce material prebuild count
    uint32 size = uint32(renderBatchArray.size());
    for (uint32 k = 0; k < size; ++k)
    {
        RenderBatch* batch = renderBatchArray[k].renderBatch;
        NMaterial* material = batch->GetMaterial();
        if (!material->HasLocalFlag(FLAG_WIND_ANIMATION))
            material->AddFlag(FLAG_WIND_ANIMATION, 1);
        else
            material->SetFlag(FLAG_WIND_ANIMATION, 1);
    }
}

AABBox3 SpeedTreeObject::CalcBBoxForSpeedTreeGeometry(RenderBatch* rb)
{
    AABBox3 pgBbox;
    PolygonGroup* pg = rb->GetPolygonGroup();

    if ((pg == nullptr) || (pg->meshData == nullptr) || (pg->GetFormat() & EVF_PIVOT4) == 0)
        return rb->GetBoundingBox();

    int32 vertexCount = pg->GetVertexCount();
    for (int32 vi = 0; vi < vertexCount; vi++)
    {
        Vector4 pivot(0.0f, 0.0f, 0.0f, 0.0f);
        if (pg->GetFormat() & EVF_PIVOT4)
        {
            pg->GetPivot(vi, pivot);
        }
        else if (pg->GetFormat() & EVF_PIVOT_DEPRECATED)
        {
            pg->GetPivotDeprecated(vi, pivot.GetVector3());
        }

        if (pivot.w > 0.f)
        {
            Vector3 pointX, pointY, pointZ;
            Vector3 offsetX, offsetY;

            pg->GetCoord(vi, pointZ);
            offsetX = offsetY = pointZ - Vector3(pivot);

            Swap(offsetX.x, offsetX.z);
            Swap(offsetX.y, offsetX.z);

            pointX = Vector3(pivot) + offsetX;
            pointY = Vector3(pivot) + offsetY;

            pgBbox.AddPoint(pointX);
            pgBbox.AddPoint(pointY);
            pgBbox.AddPoint(pointZ);
        }
        else
        {
            Vector3 position;
            pg->GetCoord(vi, position);
            pgBbox.AddPoint(position);
        }
    }

    return pgBbox;
}

Vector3 SpeedTreeObject::GetSortingDirection(uint32 directionIndex)
{
    float32 angle = (PI_2 / SpeedTreeObject::SORTING_DIRECTION_COUNT) * directionIndex;
    return Vector3(std::cos(angle), std::sin(angle), 0.f);
}

uint32 SpeedTreeObject::SelectDirectionIndex(const Vector3& direction)
{
    uint32 index = 0;
    float32 dp0 = -1.f;
    for (uint32 i = 0; i < SORTING_DIRECTION_COUNT; ++i)
    {
        float32 dp = GetSortingDirection(i).DotProduct(direction);
        if (dp > dp0)
        {
            dp0 = dp;
            index = i;
        }
    }

    return index;
}

PolygonGroup* SpeedTreeObject::CreateSortedPolygonGroup(PolygonGroup* pg)
{
    DVASSERT(pg->GetPrimitiveType() == rhi::PRIMITIVE_TRIANGLELIST);

    uint32 meshIndexCount = pg->GetPrimitiveCount() * 3;

    PolygonGroup* spg = new PolygonGroup();
    spg->AllocateData(pg->GetFormat(), pg->GetVertexCount(), meshIndexCount * SORTING_DIRECTION_COUNT, pg->GetPrimitiveCount());
    Memcpy(spg->meshData, pg->meshData, pg->GetVertexCount() * pg->vertexStride);

    for (uint32 dir = 0; dir < SpeedTreeObject::SORTING_DIRECTION_COUNT; ++dir)
    {
        Vector<uint16> bufferData = MeshUtils::BuildSortedIndexBufferData(pg, SpeedTreeObject::GetSortingDirection(dir));
        Memcpy(spg->indexArray + meshIndexCount * dir, bufferData.data(), bufferData.size() * sizeof(uint16));
    }

    spg->RecalcAABBox();
    spg->BuildBuffers();

    return spg;
}
};
