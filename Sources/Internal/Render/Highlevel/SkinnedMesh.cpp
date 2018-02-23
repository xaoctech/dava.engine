#include "FileSystem/KeyedArchive.h"
#include "Render/Highlevel/SkinnedMesh.h"
#include "Scene3D/SceneFile/SerializationContext.h"

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

    SkinnedMesh* mesh = static_cast<SkinnedMesh*>(newObject);
    uint32 batchCount = mesh->GetRenderBatchCount();
    for (uint32 ri = 0; ri < batchCount; ++ri)
    {
        RenderBatch* batch = mesh->GetRenderBatch(ri);
        RenderBatch* batch0 = GetRenderBatch(ri);

        mesh->SetJointTargets(batch, GetJointTargets(batch0));
    }

    return newObject;
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

}
