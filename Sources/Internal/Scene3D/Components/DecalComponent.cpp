#include "Scene3D/Components/DecalComponent.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(DecalComponent)
{
    ReflectionRegistrator<DecalComponent>::Begin()
    .ConstructorByPointer()
    .Field("material", &DecalComponent::GetMaterial, &DecalComponent::SetMaterial)[M::MaterialType(NMaterial::eType::TYPE_DECAL), M::DisplayName("Material")]
    .Field("DecalSize", &DecalComponent::GetLocalSize, &DecalComponent::SetLocalSize)[M::DisplayName("Decal Size")]
    .Field("sortingOffset", &DecalComponent::GetSortingOffset, &DecalComponent::SetSortingOffset)[M::Range(0, 15, 1), M::DisplayName("Sorting")]
    .End();
}

DecalComponent::DecalComponent()
{
}

Component* DecalComponent::Clone(Entity* toEntity)
{
    DecalComponent* newComponent = new DecalComponent();
    newComponent->SetEntity(toEntity);
    newComponent->decalSize = decalSize;
    newComponent->material = material;
    return newComponent;
}
void DecalComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
    if (NULL != archive)
    {
        archive->SetVariant("decal.decalSize", VariantType(decalSize));
        if (material)
        {
            uint64 matKey = material->GetNodeID();
            archive->SetUInt64("decal.material", matKey);
        }
    }
}
void DecalComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    if (NULL != archive)
    {
        decalSize = archive->GetVariant("decal.decalSize")->AsVector3();
        int64 matKey = archive->GetUInt64("decal.material");
        material = static_cast<NMaterial*>(serializationContext->GetDataBlock(matKey));
    }
    Component::Deserialize(archive, serializationContext);
}
void DecalComponent::GetDataNodes(Set<DataNode*>& dataNodes)
{
    NMaterial* curNode = material.Get();
    while (curNode != NULL)
    {
        dataNodes.insert(curNode);
        curNode = curNode->GetParent();
    }
}

void DecalComponent::SetLocalSize(const Vector3& newSize)
{
    decalSize = newSize;
    GlobalEventSystem::Instance()->Event(this, EventSystem::DECAL_COMPONENT_CHANGED);
}

const Vector3& DecalComponent::GetLocalSize() const
{
    return decalSize;
}

DecalRenderObject* DecalComponent::GetRenderObject() const
{
    return renderObject;
}

void DecalComponent::SetMaterial(NMaterial* material_)
{
    material = material_;
    GlobalEventSystem::Instance()->Event(this, EventSystem::DECAL_COMPONENT_CHANGED);
}

NMaterial* DecalComponent::GetMaterial() const
{
    return material.Get();
}

void DecalComponent::SetSortingOffset(uint32 offset)
{
    sortingOffset = offset;
    GlobalEventSystem::Instance()->Event(this, EventSystem::DECAL_COMPONENT_CHANGED);
}
uint32 DecalComponent::GetSortingOffset() const
{
    return sortingOffset;
}
}