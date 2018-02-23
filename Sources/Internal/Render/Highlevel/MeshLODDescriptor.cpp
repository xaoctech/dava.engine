#include "Asset/AssetManager.h"
#include "Engine/Engine.h"
#include "FileSystem/KeyedArchive.h"
#include "Render/Highlevel/MeshLODDescriptor.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{
void MeshLODDescriptor::Serialize(const Vector<MeshLODDescriptor>& meshLODDescriptors, KeyedArchive* archive, SerializationContext* serializationContext)
{
    uint32 lodCount = uint32(meshLODDescriptors.size());
    archive->SetUInt32("LODDescriptorCount", lodCount);
    for (uint32 l = 0; l < lodCount; ++l)
    {
        const MeshLODDescriptor& lodDesc = meshLODDescriptors[l];

        ScopedPtr<KeyedArchive> lodDescriptorArchive(new KeyedArchive());
        lodDescriptorArchive->SetString("GeometryPath", lodDesc.geometryPath.GetRelativePathname(serializationContext->GetScenePath()));
        lodDescriptorArchive->SetInt32("LODIndex", lodDesc.lodIndex);

        uint32 batchCount = uint32(lodDesc.batchDescriptors.size());
        lodDescriptorArchive->SetUInt32("BatchCount", batchCount);
        for (uint32 b = 0; b < batchCount; ++b)
        {
            const MeshBatchDescriptor& batchDesc = lodDesc.batchDescriptors[b];

            ScopedPtr<KeyedArchive> batchDescriptorArchive(new KeyedArchive());
            batchDescriptorArchive->SetString("MaterialPath", batchDesc.materialPath.GetRelativePathname(serializationContext->GetScenePath()));
            batchDescriptorArchive->SetInt32("SwitchIndex", batchDesc.switchIndex);
            batchDescriptorArchive->SetUInt32("GeometryIndex", batchDesc.geometryIndex);

            if (!batchDesc.jointTargets.empty())
            {
                uint32 jointTargetsCount = uint32(batchDesc.jointTargets.size());
                batchDescriptorArchive->SetUInt32("JointTargetsCount", jointTargetsCount);
                batchDescriptorArchive->SetByteArray("JointTargets", reinterpret_cast<const uint8*>(batchDesc.jointTargets.data()), jointTargetsCount * sizeof(int32));
            }

            lodDescriptorArchive->SetArchive(Format("BatchDescriptor%u", b), batchDescriptorArchive);
        }

        archive->SetArchive(Format("LODDescriptor%u", l), lodDescriptorArchive);
    }
}

void MeshLODDescriptor::Deserialize(Vector<MeshLODDescriptor>* meshLODDescriptors, KeyedArchive* archive, SerializationContext* serializationContext)
{
    uint32 lodCount = archive->GetUInt32("LODDescriptorCount");
    meshLODDescriptors->resize(lodCount);
    for (uint32 l = 0; l < lodCount; ++l)
    {
        MeshLODDescriptor& lodDesc = meshLODDescriptors->at(l);

        KeyedArchive* lodDescriptorArchive = archive->GetArchive(Format("LODDescriptor%u", l));
        if (lodDescriptorArchive != nullptr)
        {
            lodDesc.geometryPath = serializationContext->GetScenePath() + lodDescriptorArchive->GetString("GeometryPath");
            lodDesc.geometryAsset = GetEngineContext()->assetManager->LoadAsset<Geometry>(lodDesc.geometryPath);

            lodDesc.lodIndex = lodDescriptorArchive->GetInt32("LODIndex");

            uint32 batchCount = lodDescriptorArchive->GetUInt32("BatchCount");
            lodDesc.batchDescriptors.resize(batchCount);
            for (uint32 b = 0; b < batchCount; ++b)
            {
                MeshBatchDescriptor& batchDesc = lodDesc.batchDescriptors[b];

                KeyedArchive* batchDescriptorArchive = lodDescriptorArchive->GetArchive(Format("BatchDescriptor%u", b));
                if (batchDescriptorArchive != nullptr)
                {
                    batchDesc.materialPath = serializationContext->GetScenePath() + batchDescriptorArchive->GetString("MaterialPath");
                    batchDesc.materialAsset = GetEngineContext()->assetManager->LoadAsset<Material>(batchDesc.materialPath);

                    batchDesc.switchIndex = batchDescriptorArchive->GetInt32("SwitchIndex");
                    batchDesc.geometryIndex = batchDescriptorArchive->GetUInt32("GeometryIndex");

                    uint32 jointTargetsCount = batchDescriptorArchive->GetUInt32("JointTargetsCount");
                    if (jointTargetsCount != 0u)
                    {
                        const int32* jointTargets = reinterpret_cast<const int32*>(batchDescriptorArchive->GetByteArray("JointTargets"));
                        batchDesc.jointTargets = Vector<int32>(jointTargets, jointTargets + jointTargetsCount);
                    }
                }
            }
        }
    }
}

} // ns
