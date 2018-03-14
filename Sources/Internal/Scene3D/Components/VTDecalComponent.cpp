#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"
#include "Scene3D/Components/VTDecalComponent.h"
#include "Scene3D/Components/SingleComponents/VTSingleComponent.h"
#include "Scene3D/Systems/GlobalEventSystem.h"
#include "Scene3D/Systems/EventSystem.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Components/SingleComponents/VTSingleComponent.h"
#include "Render/Material/FXAsset.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(VTDecalComponent)
{
    ReflectionRegistrator<VTDecalComponent>::Begin()
    .ConstructorByPointer()
    .Field("material", &VTDecalComponent::GetMaterial, &VTDecalComponent::SetMaterial)[M::MaterialType(FXDescriptor::eType::TYPE_DECAL_VT), M::DisplayName("Material")]
    .Field("DecalSize", &VTDecalComponent::GetLocalSize, &VTDecalComponent::SetLocalSize)[M::DisplayName("Decal Size")]
    .Field("sortingOffset", &VTDecalComponent::GetSortingOffset, &VTDecalComponent::SetSortingOffset)[M::Range(0, 15, 1), M::DisplayName("Sorting")]
    .Field("splineSegmentationDistance", &VTDecalComponent::GetSegmentationDistance, &VTDecalComponent::SetSegmentationDistance)[M::DisplayName("Segmentation distance (meters between slices)")]
    .Field("splineTextureDistance", &VTDecalComponent::GetSplineTextureDistance, &VTDecalComponent::SetSplineTextureDistance)[M::DisplayName("Spline U size (meters in texture)")]
    //for debug
    .Field("WireframeMode", [](VTDecalComponent* comp) { return comp->GetRenderObject()->GetWireframe(); },
           [](VTDecalComponent* comp, bool value) {
            comp->GetRenderObject()->SetWireframe(value); 
            if (comp->GetEntity() && comp->GetEntity()->GetScene())
            comp->GetEntity()->GetScene()->GetSingletonComponent<VTSingleComponent>()->vtDecalChanged.push_back(comp->GetEntity()); }
           )[M::DisplayName("Wireframe Mode")]
    .End();
}

VTDecalComponent::VTDecalComponent()
{
}

void VTDecalComponent::SetRenderObject(DecalRenderObject* ro)
{
    if (renderObject != ro)
    {
        SafeRelease(renderObject);
        renderObject = SafeRetain(ro);
    }
}

VTDecalComponent::~VTDecalComponent()
{
    SafeRelease(renderObject);
}

Component* VTDecalComponent::Clone(Entity* toEntity)
{
    VTDecalComponent* newComponent = new VTDecalComponent();
    newComponent->SetEntity(toEntity);
    newComponent->decalSize = decalSize;
    newComponent->material = material;
    newComponent->splineSegmentationDistance = splineSegmentationDistance;
    newComponent->splineTextureDistance = splineTextureDistance;
    return newComponent;
}
void VTDecalComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
    if (NULL != archive)
    {
        archive->SetVector3("vtdecal.decalSize", decalSize);
        archive->SetFloat("vtdecal.splinesegmentation", splineSegmentationDistance);
        archive->SetFloat("vtdecal.splineTextureDistance", splineTextureDistance);
        if (material)
        {
            uint64 matKey = material->GetNodeID();
            archive->SetUInt64("vtdecal.material", matKey);
        }
    }
}
void VTDecalComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    if (NULL != archive)
    {
        decalSize = archive->GetVector3("vtdecal.decalSize", Vector3(1.0f, 1.0f, 1.0f));
        splineSegmentationDistance = archive->GetFloat("vtdecal.splinesegmentation", 0.5f);
        splineTextureDistance = 1.0f / archive->GetFloat("vtdecal.splineTiling", 1.0f); //GFX_COMPLETE - load old spline
        splineTextureDistance = archive->GetFloat("vtdecal.splineTextureDistance", splineTextureDistance);
        int64 matKey = archive->GetUInt64("vtdecal.material");
        material = static_cast<NMaterial*>(serializationContext->GetDataBlock(matKey));
    }
    Component::Deserialize(archive, serializationContext);
}
void VTDecalComponent::GetDataNodes(Set<DataNode*>& dataNodes)
{
    NMaterial* curNode = material.Get();
    while (curNode != NULL)
    {
        dataNodes.insert(curNode);
        curNode = curNode->GetParent();
    }
}

void VTDecalComponent::SetLocalSize(const Vector3& newSize)
{
    decalSize = newSize;
    if (entity && entity->GetScene())
        entity->GetScene()->GetSingletonComponent<VTSingleComponent>()->vtDecalChanged.push_back(entity);
}

const Vector3& VTDecalComponent::GetLocalSize() const
{
    return decalSize;
}

DecalRenderObject* VTDecalComponent::GetRenderObject() const
{
    return renderObject;
}

void VTDecalComponent::SetMaterial(NMaterial* material_)
{
    material = material_;
    if (entity && entity->GetScene())
        entity->GetScene()->GetSingletonComponent<VTSingleComponent>()->vtDecalChanged.push_back(entity);
}

NMaterial* VTDecalComponent::GetMaterial() const
{
    return material.Get();
}

void VTDecalComponent::SetSortingOffset(uint32 offset)
{
    sortingOffset = offset;
    if (entity && entity->GetScene())
        entity->GetScene()->GetSingletonComponent<VTSingleComponent>()->vtDecalChanged.push_back(entity);
}
uint32 VTDecalComponent::GetSortingOffset() const
{
    return sortingOffset;
}

void VTDecalComponent::SetSegmentationDistance(float32 distance)
{
    splineSegmentationDistance = distance;
    if (entity && entity->GetScene())
        entity->GetScene()->GetSingletonComponent<VTSingleComponent>()->vtSplineChanged.push_back(entity);
}
float32 VTDecalComponent::GetSegmentationDistance() const
{
    return splineSegmentationDistance;
}

void VTDecalComponent::SetSplineTextureDistance(float32 tiling)
{
    splineTextureDistance = tiling;
    if (entity && entity->GetScene())
        entity->GetScene()->GetSingletonComponent<VTSingleComponent>()->vtSplineChanged.push_back(entity);
}
float32 VTDecalComponent::GetSplineTextureDistance() const
{
    return splineTextureDistance;
}
}