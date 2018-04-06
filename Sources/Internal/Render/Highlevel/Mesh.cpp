#include "Render/3D/PolygonGroup.h"
#include "Render/Highlevel/Mesh.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Render/Highlevel/MeshLODDescriptor.h"
#include "Render/Material/NMaterial.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/SkeletonAnimation/JointTransform.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(Mesh)
{
    ReflectionRegistrator<Mesh>::Begin()
    .End();
}

Mesh::Mesh()
{
    type = TYPE_MESH;
}

RenderObject* Mesh::Clone(RenderObject* newObject)
{
    if (!newObject)
    {
        DVASSERT(IsPointerToExactClass<Mesh>(this), "Can clone only Mesh");
        newObject = new Mesh();
    }

    RenderObject::Clone(newObject);

    Mesh* mesh = static_cast<Mesh*>(newObject);
    for (uint32 ri = 0; ri < mesh->GetRenderBatchCount(); ++ri)
    {
        mesh->SetJointTargets(mesh->GetRenderBatch(ri), GetJointTargets(GetRenderBatch(ri)));
    }

    return newObject;
}

void Mesh::SetBoundingBox(const AABBox3& box)
{
    bbox = box;
}

void Mesh::BakeGeometry(const Matrix4& transform)
{
    uint32 size = static_cast<uint32>(renderBatchArray.size());
    for (uint32 i = 0; i < size; ++i)
    {
        PolygonGroup* pg = renderBatchArray[i].renderBatch->GetPolygonGroup();
        DVASSERT(pg);
        pg->ApplyMatrix(transform);
        pg->BuildBuffers();

        renderBatchArray[i].renderBatch->UpdateAABBoxFromSource();
    }

    RecalcBoundingBox();
}

void Mesh::UpdateJointTransforms(const Vector<JointTransform>& finalTransforms)
{
    for (auto& jointsData : jointTargetsData)
    {
        const JointTargets& targets = jointsData.first;
        JointTargetsData& data = jointsData.second;

        for (uint32 j = 0; j < data.jointsDataCount; ++j)
        {
            uint32 transformIndex = targets[j];
            DVASSERT(transformIndex < uint32(finalTransforms.size()));

            const JointTransform& finalTransform = finalTransforms[transformIndex];
            data.positions[j] = Vector4(Vector3(finalTransform.GetPosition().data), finalTransform.GetScale());
            data.quaternions[j] = Vector4(finalTransform.GetOrientation().data);
        }
    }
}

bool Mesh::HasSkinnedBatches() const
{
    return !jointTargetsDataMap.empty();
}

const Mesh::JointTargets& Mesh::GetJointTargets(RenderBatch* batch)
{
    auto found = jointTargetsDataMap.find(batch);
    if (found != jointTargetsDataMap.end())
        return jointTargetsData[found->second].first;

    static const JointTargets emptyJointTargets;
    return emptyJointTargets;
}

const Mesh::JointTargetsData& Mesh::GetJointTargetsData(RenderBatch* batch)
{
    auto found = jointTargetsDataMap.find(batch);
    if (found != jointTargetsDataMap.end())
    {
        uint32 dataIndex = found->second;
        return jointTargetsData[dataIndex].second;
    }

    static const JointTargetsData emptyJointTargetsData = JointTargetsData();
    return emptyJointTargetsData;
}

void Mesh::SetJointTargets(RenderBatch* batch, const JointTargets& targets)
{
    DVASSERT(uint32(targets.size()) <= MAX_TARGET_JOINTS);

    auto found = std::find_if(jointTargetsData.begin(), jointTargetsData.end(), [&targets](const std::pair<JointTargets, JointTargetsData>& item) {
        return (item.first == targets);
    });

    if (found != jointTargetsData.end())
    {
        jointTargetsDataMap[batch] = uint32(std::distance(jointTargetsData.begin(), found));
    }
    else
    {
        uint32 dataIndex = uint32(jointTargetsData.size());
        jointTargetsData.emplace_back();

        uint32 targetsCount = uint32(targets.size());
        jointTargetsData.back().first = targets;
        jointTargetsData.back().second.positions.resize(targetsCount);
        jointTargetsData.back().second.quaternions.resize(targetsCount);
        jointTargetsData.back().second.jointsDataCount = targetsCount;

        prevJointTargetsData.back().first = targets;
        prevJointTargetsData.back().second.positions.resize(targetsCount);
        prevJointTargetsData.back().second.quaternions.resize(targetsCount);
        prevJointTargetsData.back().second.jointsDataCount = targetsCount;

        jointTargetsDataMap[batch] = dataIndex;
    }
}

void Mesh::UpdatePreviousState()
{
    for (uint32 i = 0; i < jointTargetsData.size(); ++i)
    {
        DVASSERT(jointTargetsData.size() == prevJointTargetsData.size());

        JointTargetsData& data = jointTargetsData[i].second;
        JointTargetsData& prevData = prevJointTargetsData[i].second;

        for (uint32 j = 0; j < data.jointsDataCount; ++j)
        {
            DVASSERT(prevData.positions.size() == data.positions.size());
            DVASSERT(prevData.quaternions.size() == data.quaternions.size());

            prevData.positions[j] = data.positions[j];
            prevData.quaternions[j] = data.quaternions[j];
        }
    }

    RenderObject::UpdatePreviousState();
}

void Mesh::UpdateJointsTransformsProperties()
{
    for (const RenderBatchWithOptions& rb : renderBatchArray)
    {
        auto found = jointTargetsDataMap.find(rb.renderBatch);
        if (found != jointTargetsDataMap.end())
        {
            const JointTargetsData& data = jointTargetsData[found->second].second;
            const JointTargetsData& prevData = prevJointTargetsData[found->second].second;

            SetDynamicProperty(rb.renderBatch, DynamicBindings::PARAM_JOINTS_COUNT, &data.jointsDataCount, reinterpret_cast<pointer_size>(&data.jointsDataCount));

            SetDynamicProperty(rb.renderBatch, DynamicBindings::PARAM_JOINT_POSITIONS, data.positions.data(), reinterpret_cast<pointer_size>(data.positions.data()));
            SetDynamicProperty(rb.renderBatch, DynamicBindings::PARAM_JOINT_QUATERNIONS, data.quaternions.data(), reinterpret_cast<pointer_size>(data.quaternions.data()));

            SetDynamicProperty(rb.renderBatch, DynamicBindings::PARAM_PREV_JOINT_POSITIONS, prevData.positions.data(), reinterpret_cast<pointer_size>(prevData.positions.data()));
            SetDynamicProperty(rb.renderBatch, DynamicBindings::PARAM_PREV_JOINT_QUATERNIONS, prevData.quaternions.data(), reinterpret_cast<pointer_size>(prevData.quaternions.data()));
        }
        else
        {
            RemoveDynamicProperty(rb.renderBatch, DynamicBindings::PARAM_JOINTS_COUNT);
            RemoveDynamicProperty(rb.renderBatch, DynamicBindings::PARAM_JOINT_POSITIONS);
            RemoveDynamicProperty(rb.renderBatch, DynamicBindings::PARAM_JOINT_QUATERNIONS);
            RemoveDynamicProperty(rb.renderBatch, DynamicBindings::PARAM_PREV_JOINT_POSITIONS);
            RemoveDynamicProperty(rb.renderBatch, DynamicBindings::PARAM_PREV_JOINT_QUATERNIONS);
        }
    }
}

void Mesh::AddMeshBatches(const Vector<MeshLODDescriptor>& desc)
{
    for (const MeshLODDescriptor& lodDesc : desc)
    {
        for (const MeshBatchDescriptor& batchDesc : lodDesc.batchDescriptors)
        {
            ScopedPtr<RenderBatch> batch(new RenderBatch());
            batch->SetPolygonGroup(lodDesc.geometryAsset->GetPolygonGroup(batchDesc.geometryIndex));
            batch->SetMaterial(batchDesc.materialAsset->GetMaterial());

            AddRenderBatch(batch, lodDesc.lodIndex, batchDesc.switchIndex);

            if (!batchDesc.jointTargets.empty())
                SetJointTargets(batch, batchDesc.jointTargets);
        }
    }

    UpdateJointsTransformsProperties();
}

//DEPRECATED
void Mesh::Load(KeyedArchive* archive, SerializationContext* serializationContext)
{
    RenderObject::Load(archive, serializationContext);
}

void Mesh::AddPolygonGroup(PolygonGroup* polygonGroup, NMaterial* material)
{
    RenderBatch* batch = new RenderBatch();
    batch->SetPolygonGroup(polygonGroup);
    batch->SetMaterial(material);
    batch->indexCount = polygonGroup->GetPrimitiveCount() * 3;
    AddRenderBatch(batch);

    batch->Release();
}
};
