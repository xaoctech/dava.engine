#include "Asset/AssetManager.h"
#include "Engine/Engine.h"
#include "Scene3D/Components/MeshComponent.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"
#include "Render/Highlevel/Mesh.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(MeshComponent)
{
    ReflectionRegistrator<MeshComponent>::Begin()[M::CantBeCreatedManualyComponent()]
    .ConstructorByPointer()
    .Field("meshDescriptors", &MeshComponent::meshLODDescriptors)[M::DisplayName("LOD Descriptors")]
    .Field("mesh", &MeshComponent::mesh)[M::DisplayName("Mesh")]
    .End();
}

MeshComponent::MeshComponent()
{
    mesh = new Mesh();
    listener.onReloaded = MakeFunction(this, &MeshComponent::OnAssetReloaded);
}

MeshComponent::~MeshComponent()
{
    AssetManager* assetManager = GetEngineContext()->assetManager;
    assetManager->UnregisterListener(&listener);
    SafeRelease(mesh);
}

void MeshComponent::SetMeshDescriptor(const Vector<MeshLODDescriptor>& desc)
{
    AssetManager* assetManager = GetEngineContext()->assetManager;
    for (MeshLODDescriptor& descr : meshLODDescriptors)
    {
        assetManager->UnregisterListener(descr.geometryAsset, &listener);
        for (MeshBatchDescriptor& batchDescr : descr.batchDescriptors)
        {
            assetManager->UnregisterListener(batchDescr.materialAsset, &listener);
        }
    }
    meshLODDescriptors = desc;
    for (MeshLODDescriptor& descr : meshLODDescriptors)
    {
        assetManager->RegisterListener(descr.geometryAsset, &listener);
        for (MeshBatchDescriptor& batchDescr : descr.batchDescriptors)
        {
            assetManager->RegisterListener(batchDescr.materialAsset, &listener);
        }
    }
    RebuildMesh();
}

const Vector<MeshLODDescriptor>& MeshComponent::GetMeshDescriptor() const
{
    return meshLODDescriptors;
}

Mesh* MeshComponent::GetMesh() const
{
    return mesh;
}

Component* MeshComponent::Clone(Entity* toEntity)
{
    MeshComponent* newComponent = new MeshComponent();
    newComponent->SetEntity(toEntity);
    newComponent->SetMeshDescriptor(GetMeshDescriptor());
    newComponent->mesh->SetFlags(mesh->GetFlags());

    return newComponent;
}

void MeshComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);

    MeshLODDescriptor::Serialize(meshLODDescriptors, archive, serializationContext);
    mesh->SaveFlags(archive, serializationContext);
}

void MeshComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);

    MeshLODDescriptor::Deserialize(&meshLODDescriptors, archive, serializationContext);
    mesh->LoadFlags(archive, serializationContext);

    RebuildMesh();
}

void MeshComponent::RebuildMesh()
{
    mesh->RemoveBatches();
    mesh->AddMeshBatches(meshLODDescriptors);

    if (mesh->HasSkinnedBatches())
        mesh->AddFlag(RenderObject::eFlags::CUSTOM_PREPARE_TO_RENDER);
    else
        mesh->RemoveFlag(RenderObject::eFlags::CUSTOM_PREPARE_TO_RENDER);
}

void MeshComponent::OnAssetReloaded(const Asset<AssetBase>& originalAsset, const Asset<AssetBase>& reloadedAsset)
{
    Asset<Material> material = std::dynamic_pointer_cast<Material>(reloadedAsset);
    Asset<Geometry> geometry = std::dynamic_pointer_cast<Geometry>(reloadedAsset);
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
    RebuildMesh();
}
}
