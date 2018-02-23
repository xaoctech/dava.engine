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
class DecalComponent : public Component
{
    friend class DecalSystem;

public:
    DecalComponent();

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

    DecalRenderObject* GetRenderObject() const;

private:
    Vector3 decalSize = Vector3(1.0f, 1.0f, 1.0f);
    uint32 sortingOffset = 7;
    RefPtr<NMaterial> material;

    //GFX_COMPLETE -- test
    DecalRenderObject* renderObject = nullptr;

public:
    DAVA_VIRTUAL_REFLECTION(DecalComponent, Component);
};
}