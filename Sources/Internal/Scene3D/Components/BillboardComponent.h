#pragma once

#include "Asset/AssetListener.h"
#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Reflection/Reflection.h"
#include "Render/Highlevel/MeshLODDescriptor.h"

namespace DAVA
{
class BillboardRenderObject;
class SerializationContext;
class BillboardComponent : public Component
{
protected:
    virtual ~BillboardComponent();

public:
    BillboardComponent();

    void Rebuild();
    void SetMeshDescriptor(const Vector<MeshLODDescriptor>& desc);
    const Vector<MeshLODDescriptor>& GetMeshDescriptor() const;

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void GetDataNodes(Set<DataNode*>& dataNodes) override;

    BillboardRenderObject* GetBillboard() const;

protected:
    void RebuildMesh();
    void OnAssetReloaded(const Asset<AssetBase>& originalAsset, const Asset<AssetBase>& reloadedAsset);

    Vector<MeshLODDescriptor> meshLODDescriptors;

    //runtime
    BillboardRenderObject* renderObject = nullptr;
    SimpleAssetListener listener;

    DAVA_VIRTUAL_REFLECTION(BillboardComponent, Component);
};
}
