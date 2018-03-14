#pragma once

#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
#include "Entity/Component.h"
#include "Render/Highlevel/MeshLODDescriptor.h"
#include "Asset/AssetListener.h"

namespace DAVA
{
class Mesh;
class SerializationContext;
class MeshComponent : public Component
{
protected:
    virtual ~MeshComponent();

public:
    MeshComponent();

    void SetMeshDescriptor(const Vector<MeshLODDescriptor>& desc);
    const Vector<MeshLODDescriptor>& GetMeshDescriptor() const;

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    Mesh* GetMesh() const;

protected:
    void RebuildMesh();
    void OnAssetReloaded(const Asset<AssetBase>& originalAsset, const Asset<AssetBase>& reloadedAsset);

    Vector<MeshLODDescriptor> meshLODDescriptors;

    //runtime
    Mesh* mesh = nullptr;
    SimpleAssetListener listener;

    DAVA_VIRTUAL_REFLECTION(MeshComponent, Component);
};
}
