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
}

MeshComponent::~MeshComponent()
{
    SafeRelease(mesh);
}

void MeshComponent::SetMeshDescriptor(const Vector<MeshLODDescriptor>& desc)
{
    meshLODDescriptors = desc;
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

}
