#include "Scene3D/Components/SpeedTreeComponent.h"

#include "Asset/AssetManager.h"
#include "FileSystem/KeyedArchive.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"
#include "Render/3D/Geometry.h"
#include "Render/Highlevel/SpeedTreeObject.h"
#include "Render/Material/Material.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Utils/StringFormat.h"

namespace DAVA
{
const float defaultFlexibilityValues[SpeedTreeComponent::Element::Count] =
{
  0.01f, 0.025f, 0.05f, 1.0f
};
const float defaultTrunkDamping = 5.0f; // beacause why not?

DAVA_VIRTUAL_REFLECTION_IMPL(SpeedTreeComponent)
{
    ReflectionRegistrator<SpeedTreeComponent>::Begin()[M::CantBeCreatedManualyComponent()]
    .ConstructorByPointer()
    .Field("speedTreeObject", &SpeedTreeComponent::treeObject)[M::DisplayName("Speed Tree Object")]
    .Field("trunkFlexibility", &SpeedTreeComponent::GetTrunkFlexibility, &SpeedTreeComponent::SetTrunkFlexibility)[M::DisplayName("Trunk Flexibility")]
    .Field("branchFlexibility", &SpeedTreeComponent::GetBranchesFlexibility, &SpeedTreeComponent::SetBranchesFlexibility)[M::DisplayName("Branch Flexibility")]
    .Field("leafsFlexibility", &SpeedTreeComponent::GetLeavesFlexibility, &SpeedTreeComponent::SetLeavesFlexibility)[M::DisplayName("Leaves Flexibility")]
    .Field("trunkDamping", &SpeedTreeComponent::trunkDamping)[M::DisplayName("Trunk Damping")]
    .End();
}

SpeedTreeComponent::SpeedTreeComponent()
{
    treeObject = new SpeedTreeObject();

    for (uint32 i = 0; i < Element::Count; ++i)
        flexibility[i] = defaultFlexibilityValues[i];
    trunkDamping = defaultTrunkDamping;
}

SpeedTreeComponent::~SpeedTreeComponent()
{
    AssetManager* assetManager = GetEngineContext()->assetManager;
    assetManager->UnregisterListener(this);
    SafeRelease(treeObject);
}

Component* SpeedTreeComponent::Clone(Entity* toEntity)
{
    SpeedTreeComponent* component = new SpeedTreeComponent();
    component->SetEntity(toEntity);
    component->bones = bones;
    component->runtime = runtime;
    component->trunkDamping = trunkDamping;
    memcpy(component->flexibility, flexibility, sizeof(flexibility));
    component->SetMeshDescriptor(GetMeshDescriptor());
    component->GetSpeedTreeObject()->SetFlags(GetSpeedTreeObject()->GetFlags());

    return component;
}

void SpeedTreeComponent::SetMeshDescriptor(const Vector<MeshLODDescriptor>& desc)
{
    AssetManager* assetManager = GetEngineContext()->assetManager;
    for (MeshLODDescriptor& descr : meshLODDescriptors)
    {
        assetManager->UnregisterListener(descr.geometryAsset, this);
        for (MeshBatchDescriptor& batchDescr : descr.batchDescriptors)
        {
            assetManager->UnregisterListener(batchDescr.materialAsset, this);
        }
    }
    meshLODDescriptors = desc;
    for (MeshLODDescriptor& descr : meshLODDescriptors)
    {
        assetManager->RegisterListener(descr.geometryAsset, this);
        for (MeshBatchDescriptor& batchDescr : descr.batchDescriptors)
        {
            assetManager->RegisterListener(batchDescr.materialAsset, this);
        }
    }
    RebuildObject();
}

const Vector<MeshLODDescriptor>& SpeedTreeComponent::GetMeshDescriptor() const
{
    return meshLODDescriptors;
}

void SpeedTreeComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    DVASSERT(archive != nullptr);

    Component::Serialize(archive, serializationContext);

    MeshLODDescriptor::Serialize(meshLODDescriptors, archive, serializationContext);
    treeObject->SaveFlags(archive, serializationContext);

    uint32 bonesCount = static_cast<uint32>(bones.size());
    archive->SetUInt32("bones.count", bonesCount);
    for (uint32 i = 0; i < bonesCount; ++i)
    {
        ScopedPtr<KeyedArchive> bone(new KeyedArchive);
        bones[i].Serialize(bone);
        archive->SetArchive(Format("bone.%u", i), bone);
    }

    for (uint32 i = 0; i < Element::Count; ++i)
    {
        archive->SetFloat(Format("flexibility.%u", i), flexibility[i]);
    }

    archive->SetFloat("trunkDamping", trunkDamping);
}

void SpeedTreeComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    DVASSERT(archive != nullptr);

    Component::Deserialize(archive, serializationContext);
    treeObject->LoadFlags(archive, serializationContext);

    MeshLODDescriptor::Deserialize(&meshLODDescriptors, archive, serializationContext);

    uint32 bonesCount = archive->GetUInt32("bones.count", 0);
    for (uint32_t i = 0; i < bonesCount; ++i)
    {
        KeyedArchive* bone = archive->GetArchive(Format("bone.%u", i));
        bones.emplace_back();
        bones.back().Deserialize(bone);
    }

    for (uint32 i = 0; i < Element::Count; ++i)
    {
        flexibility[i] = archive->GetFloat(Format("flexibility.%u", i), defaultFlexibilityValues[i]);
    }

    trunkDamping = archive->GetFloat("trunkDamping", defaultTrunkDamping);

    runtime.resize(bones.size());

    RebuildObject();
}

void SpeedTreeComponent::SetBones(const Vector<SpeedTreeComponent::Bone>& bones_)
{
    bones = bones_;
    runtime.resize(bones.size());
}

void SpeedTreeComponent::Bone::Serialize(KeyedArchive* archive)
{
    DVASSERT(archive != nullptr);

    archive->SetInt32("id", id);
    archive->SetInt32("parentId", parentId);
    archive->SetUInt32("generator", static_cast<uint32>(element));
    archive->SetVector3("start", start);
    archive->SetVector3("end", end);
    archive->SetFloat("mass", mass);
    archive->SetFloat("massWithChildren", massWithChildren);
    archive->SetFloat("radius", radius);
}

void SpeedTreeComponent::Bone::Deserialize(KeyedArchive* archive)
{
    if (archive == nullptr)
        return;

    id = archive->GetInt32("id", id);
    parentId = archive->GetInt32("parentId", parentId);
    start = archive->GetVector3("start", start);
    end = archive->GetVector3("end", end);
    mass = archive->GetFloat("mass", mass);
    massWithChildren = archive->GetFloat("massWithChildren", massWithChildren);
    radius = archive->GetFloat("radius", radius);

    uint32 gen = archive->GetUInt32("generator", 0);
    if (gen < static_cast<uint32>(Element::Count))
        element = static_cast<Element>(gen);
}

SpeedTreeObject* SpeedTreeComponent::GetSpeedTreeObject() const
{
    return treeObject;
}

void SpeedTreeComponent::RebuildObject()
{
    treeObject->RemoveBatches();
    treeObject->AddMeshBatches(meshLODDescriptors);

    treeObject->SetDynamicProperty(DynamicBindings::PARAM_WIND, wind.data, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
    treeObject->SetDynamicProperty(DynamicBindings::PARAM_PREV_WIND, prevWind.data, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
    treeObject->SetDynamicProperty(DynamicBindings::PARAM_FLEXIBILITY, leafsFlexibility.data, DynamicBindings::UPDATE_SEMANTIC_ALWAYS);
}

void SpeedTreeComponent::OnAssetReloaded(const Asset<AssetBase>& originalAsset, const Asset<AssetBase>& reloaded)
{
    Asset<Material> material = std::dynamic_pointer_cast<Material>(reloaded);
    Asset<Geometry> geometry = std::dynamic_pointer_cast<Geometry>(reloaded);
    for (MeshLODDescriptor& descr : meshLODDescriptors)
    {
        if (geometry != nullptr && descr.geometryAsset == originalAsset)
        {
            descr.geometryAsset = geometry;
        }

        if (material != nullptr)
        {
            for (MeshBatchDescriptor& batchDescr : descr.batchDescriptors)
            {
                if (batchDescr.materialAsset == originalAsset)
                {
                    batchDescr.materialAsset = material;
                }
            }
        }
    }

    RebuildObject();
}
};
