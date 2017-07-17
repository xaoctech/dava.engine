#include "Render/Highlevel/SkinnedMesh.h"
#include "Render/Renderer.h"

namespace DAVA
{
SkinnedMesh::SkinnedMesh()
{
    type = TYPE_SKINNED_MESH;
    flags |= RenderObject::eFlags::CUSTOM_PREPARE_TO_RENDER;
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

void SkinnedMesh::Save(KeyedArchive* archive, SerializationContext* serializationContext)
{
    uint32 rbCount = GetRenderBatchCount();
    for (uint32 r = 0; r < rbCount; ++r)
    {
        RenderBatch* batch = GetRenderBatch(r);
        if (jointsMapping.count(batch))
        {
            const Vector<int32>& jMapping = jointsMapping[batch];

            uint32 mappingSize = uint32(jMapping.size());
            archive->SetUInt32(Format("skinnedObject.batch%d.jointMappingCount", r), mappingSize);
            for (uint32 m = 0; m < mappingSize; ++m)
                archive->SetInt32(Format("skinnedObject.batch%d.jointMapping%d", r, m), jMapping[m]);
        }
    }

    RenderObject::Save(archive, serializationContext);
}

void SkinnedMesh::Load(KeyedArchive* archive, SerializationContext* serializationContext)
{
    RenderObject::Load(archive, serializationContext);

    uint32 rbCount = GetRenderBatchCount();
    for (uint32 r = 0; r < rbCount; ++r)
    {
        RenderBatch* batch = GetRenderBatch(r);

        if (archive->IsKeyExists(Format("skinnedObject.batch%d.jointMappingCount", r)))
        {
            uint32 mappingSize = archive->GetUInt32(Format("skinnedObject.batch%d.jointMappingCount", r));
            jointsMapping[batch] = Vector<int32>(mappingSize);
            for (uint32 m = 0; m < mappingSize; ++m)
                jointsMapping[batch][m] = archive->GetInt32(Format("skinnedObject.batch%d.jointMapping%d", r, m), -1);

            jointsData[batch].jointsDataCount = mappingSize;
            jointsData[batch].positions.resize(mappingSize);
            jointsData[batch].quaternions.resize(mappingSize);
        }
    }
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

void SkinnedMesh::BindDynamicParameters(Camera* camera, RenderBatch* batch)
{
    if (jointsMapping.empty())
    {
        DVASSERT(skeletonJointCount <= MAX_TARGET_JOINTS);

        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_JOINTS_COUNT, &skeletonJointCount, reinterpret_cast<pointer_size>(&skeletonJointCount));
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_JOINT_POSITIONS, positionArray, reinterpret_cast<pointer_size>(positionArray));
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_JOINT_QUATERNIONS, quaternionArray, reinterpret_cast<pointer_size>(quaternionArray));
    }
    else
    {
        const BatchJointData& data = jointsData[batch];

        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_JOINTS_COUNT, &data.jointsDataCount, reinterpret_cast<pointer_size>(&data.jointsDataCount));
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_JOINT_POSITIONS, data.positions.data(), reinterpret_cast<pointer_size>(data.positions.data()));
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_JOINT_QUATERNIONS, data.quaternions.data(), reinterpret_cast<pointer_size>(data.quaternions.data()));
    }

    RenderObject::BindDynamicParameters(camera, batch);
}

void SkinnedMesh::PrepareToRender(Camera* camera)
{
    if (!jointsMapping.empty())
    {
        for (RenderBatch* b : activeRenderBatchArray)
        {
            const Vector<int32>& jMapping = jointsMapping[b];
            BatchJointData& data = jointsData[b];

            for (uint32 j = 0; j < data.jointsDataCount; ++j)
            {
                data.positions[j] = positionArray[jMapping[j]];
                data.quaternions[j] = quaternionArray[jMapping[j]];
            }
        }
    }
}
}