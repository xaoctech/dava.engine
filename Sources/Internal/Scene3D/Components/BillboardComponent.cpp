#include "Scene3D/Components/BillboardComponent.h"

#include "Asset/AssetManager.h"
#include "Engine/Engine.h"
#include "Scene3D/Components/SingleComponents/RenderObjectSingleComponent.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Render/Highlevel/BillboardRenderObject.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(BillboardComponent)
{
    ReflectionRegistrator<BillboardComponent>::Begin()[M::DisplayName("Billboard component")]
    .ConstructorByPointer()
    .Field("type", [](BillboardComponent* obj) -> BillboardRenderObject::BillboardType {
        return static_cast<BillboardRenderObject::BillboardType>(obj->renderObject->GetBillboardType());
    },
           [](BillboardComponent* obj, BillboardRenderObject::BillboardType type) {
               obj->renderObject->SetBillboardType(type);
           })[M::DisplayName("Billboard Type"), M::EnumT<BillboardRenderObject::BillboardType>()]
    .Field("meshDescriptors", &BillboardComponent::meshLODDescriptors)[M::DisplayName("LODs")]
    .End();
}

BillboardComponent::BillboardComponent()
{
    renderObject = new BillboardRenderObject();
    listener.onReloaded = MakeFunction(this, &BillboardComponent::OnAssetReloaded);
}

void BillboardComponent::Rebuild()
{
    AssetManager* assetManager = GetEngineContext()->assetManager;
    assetManager->UnregisterListener(&listener);

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

BillboardComponent::~BillboardComponent()
{
    AssetManager* assetManager = GetEngineContext()->assetManager;
    assetManager->UnregisterListener(&listener);
    SafeRelease(renderObject);
}

void BillboardComponent::SetMeshDescriptor(const Vector<MeshLODDescriptor>& desc)
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
        if (descr.geometryAsset == nullptr)
        {
            descr.geometryAsset = assetManager->GetAsset<Geometry>(Geometry::PathKey(descr.geometryPath), AssetManager::SYNC);
        }
    }
    Rebuild();
}

const Vector<MeshLODDescriptor>& BillboardComponent::GetMeshDescriptor() const
{
    return meshLODDescriptors;
}

BillboardRenderObject* BillboardComponent::GetBillboard() const
{
    return renderObject;
}

Component* BillboardComponent::Clone(Entity* toEntity)
{
    BillboardComponent* newComponent = new BillboardComponent();
    newComponent->SetEntity(toEntity);
    newComponent->SetMeshDescriptor(GetMeshDescriptor());
    newComponent->renderObject->SetFlags(renderObject->GetFlags());

    return newComponent;
}

void BillboardComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);

    MeshLODDescriptor::Serialize(meshLODDescriptors, archive, serializationContext);
    renderObject->SaveFlags(archive, serializationContext);
    archive->SetUInt32("billboardType", renderObject->GetBillboardType());
}

void BillboardComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);

    MeshLODDescriptor::Deserialize(&meshLODDescriptors, archive, serializationContext);
    renderObject->LoadFlags(archive, serializationContext);
    renderObject->SetBillboardType(archive->GetUInt32("billboardType", BillboardRenderObject::BILLBOARD_CYLINDRICAL));

    Rebuild();
}

void BillboardComponent::GetDataNodes(Set<DataNode*>& dataNodes)
{
    for (const MeshLODDescriptor& descr : meshLODDescriptors)
    {
        for (const MeshBatchDescriptor& batchDescr : descr.batchDescriptors)
        {
            if (batchDescr.materialAsset != nullptr)
            {
                NMaterial* material = batchDescr.materialAsset->GetMaterial();
                if (material != nullptr)
                {
                    dataNodes.insert(material);
                }
            }
        }
    }
}

void BillboardComponent::RebuildMesh()
{
    renderObject->RemoveBatches();

    for (const MeshLODDescriptor& lodDesc : meshLODDescriptors)
    {
        for (const MeshBatchDescriptor& batchDesc : lodDesc.batchDescriptors)
        {
            ScopedPtr<RenderBatch> batch(new RenderBatch());
            batch->SetPolygonGroup(lodDesc.geometryAsset->GetPolygonGroup(batchDesc.geometryIndex));
            batch->SetMaterial(batchDesc.materialAsset->GetMaterial());

            renderObject->AddRenderBatch(batch, lodDesc.lodIndex, batchDesc.switchIndex);
        }
    }

    renderObject->AddFlag(RenderObject::eFlags::CUSTOM_PREPARE_TO_RENDER);
    renderObject->RecalcBoundingBox();

    if (entity != nullptr)
    {
        Scene* scene = entity->GetScene();
        if (scene != nullptr)
        {
            scene->GetSingletonComponent<RenderObjectSingleComponent>()->changedRenderObjects.emplace(renderObject, entity);
        }
    }
}

void BillboardComponent::OnAssetReloaded(const Asset<AssetBase>& originalAsset, const Asset<AssetBase>& reloadedAsset)
{
    Asset<Material> material = std::dynamic_pointer_cast<Material>(reloadedAsset);
    Asset<Geometry> geometry = std::dynamic_pointer_cast<Geometry>(reloadedAsset);
    bool rebuildMesh = false;
    for (MeshLODDescriptor& descr : meshLODDescriptors)
    {
        if (geometry != nullptr && descr.geometryAsset == originalAsset)
        {
            descr.SetGeometry(geometry);
            rebuildMesh = true;
        }

        if (material != nullptr)
        {
            for (MeshBatchDescriptor& batchDescr : descr.batchDescriptors)
            {
                if (batchDescr.materialAsset == originalAsset)
                {
                    batchDescr.materialAsset = material;
                    rebuildMesh = true;
                }
            }
        }
    }
    RebuildMesh();
}
}
