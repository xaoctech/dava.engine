#include "Scene3D/SkeletonAnimation/JointTransform.h"
#include "Render/Highlevel/SkinnedMesh.h"
#include "Render/Renderer.h"

namespace DAVA
{
SkinnedMesh::SkinnedMesh()
    : jointTargets(16)
    , jointTargetsData(16)
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

    SkinnedMesh* mesh = static_cast<SkinnedMesh*>(newObject);
    uint32 batchCount = mesh->GetRenderBatchCount();
    for (uint32 ri = 0; ri < batchCount; ++ri)
    {
        RenderBatch* batch = mesh->GetRenderBatch(ri);
        RenderBatch* batch0 = GetRenderBatch(ri);

        mesh->SetJointTargets(batch, jointTargets[batch0]);
    }

    return newObject;
}

void SkinnedMesh::Save(KeyedArchive* archive, SerializationContext* serializationContext)
{
    uint32 rbCount = GetRenderBatchCount();
    for (uint32 ri = 0; ri < rbCount; ++ri)
    {
        RenderBatch* batch = GetRenderBatch(ri);

        auto found = jointTargets.find(batch);
        if (found != jointTargets.end())
        {
            const JointTargets& targets = found->second;

            uint32 targetsCount = uint32(targets.size());
            archive->SetUInt32(Format("skinnedObject.batch%d.targetsCount", ri), targetsCount);
            archive->SetByteArray(Format("skinnedObject.batch%d.targetsData", ri), reinterpret_cast<const uint8*>(targets.data()), targetsCount * sizeof(int32));
        }
    }

    RenderObject::Save(archive, serializationContext);
}

void SkinnedMesh::Load(KeyedArchive* archive, SerializationContext* serializationContext)
{
    RenderObject::Load(archive, serializationContext);

    uint32 rbCount = GetRenderBatchCount();
    for (uint32 ri = 0; ri < rbCount; ++ri)
    {
        RenderBatch* batch = GetRenderBatch(ri);

        String targetsCountKey = Format("skinnedObject.batch%d.targetsCount", ri);
        if (archive->IsKeyExists(targetsCountKey))
        {
            uint32 targetsCount = archive->GetUInt32(targetsCountKey);
            JointTargets targets(targetsCount);
            const uint8* targetsData = archive->GetByteArray(Format("skinnedObject.batch%d.targetsData", ri));
            if (targetsData != nullptr)
                Memcpy(targets.data(), targetsData, targetsCount * sizeof(int32));

            SetJointTargets(batch, targets);
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
    auto found = jointTargetsData.find(batch);
    if (found != jointTargetsData.end())
    {
        const JointTargetsData& data = found->second;

        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_JOINTS_COUNT, &data.jointsDataCount, reinterpret_cast<pointer_size>(&data.jointsDataCount));
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_JOINT_POSITIONS, data.positions.data(), reinterpret_cast<pointer_size>(data.positions.data()));
        Renderer::GetDynamicBindings().SetDynamicParam(DynamicBindings::PARAM_JOINT_QUATERNIONS, data.quaternions.data(), reinterpret_cast<pointer_size>(data.quaternions.data()));
    }

    RenderObject::BindDynamicParameters(camera, batch);
}

void SkinnedMesh::PrepareToRender(Camera* camera)
{
    if (!jointTargets.empty())
    {
        DVASSERT(skeletonFinalJointTransforms);

        for (RenderBatch* b : activeRenderBatchArray)
        {
            auto found = jointTargets.find(b);
            if (found == jointTargets.end())
                continue;

            const JointTargets& targets = found->second;
            JointTargetsData& data = jointTargetsData[b];
            for (uint32 j = 0; j < data.jointsDataCount; ++j)
            {
                uint32 transformIndex = targets[j];
                DVASSERT(transformIndex < skeletonJointCount);

                const JointTransform& finalTransform = skeletonFinalJointTransforms[transformIndex];
                data.positions[j] = Vector4(Vector3(finalTransform.position.data), finalTransform.scale);
                data.quaternions[j] = Vector4(finalTransform.orientation.data);
            }
        }
    }
}
}