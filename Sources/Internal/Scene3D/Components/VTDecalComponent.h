#pragma once

#include "Base/BaseTypes.h"
#include "Base/BaseObject.h"
#include "Entity/Component.h"
#include "Math/AABBox3.h"
#include "Reflection/Reflection.h"
#include "Scene3D/Entity.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Render/Highlevel/DecalRenderObject.h"

namespace DAVA
{
class VTDecalComponent : public Component
{
    friend class VTDecalSystem;
    friend class VTSplineDecalSystem;

public:
    VTDecalComponent();
    ~VTDecalComponent();

    Component* Clone(Entity* toEntity) override;
    void GetDataNodes(Set<DataNode*>& dataNodes) override; //GFX_COMPLETE - once materials will be moved to MaterialLibrary no need to export data nodes here
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void SetLocalSize(const Vector3& newSize);
    const Vector3& GetLocalSize() const;

    void SetMaterial(NMaterial* material);
    NMaterial* GetMaterial() const;

    /**
    * should be within range 0..15
    */
    void SetSortingOffset(uint32 offset);
    uint32 GetSortingOffset() const;

    void SetSegmentationDistance(float32 distance);
    float32 GetSegmentationDistance() const;

    void SetSplineTextureDistance(float32 tiling);
    float32 GetSplineTextureDistance() const;

    void SetRenderObject(DecalRenderObject* renderObject);
    DecalRenderObject* GetRenderObject() const;

private:
    Vector3 decalSize = Vector3(1.0f, 1.0f, 1.0f);
    uint32 sortingOffset = 7;
    RefPtr<NMaterial> material;

    float32 splineSegmentationDistance = 0.5f; //2 slices per meter
    float32 splineTextureDistance = 1.0; //

    DecalRenderObject* renderObject = nullptr;

public:
    DAVA_VIRTUAL_REFLECTION(VTDecalComponent, Component);
};
}