#pragma once

#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
#include "Entity/Component.h"
#include "Render/Highlevel/RenderObject.h"
#include "Scene3D/SceneFile/SerializationContext.h"

namespace DAVA
{
class LightmapComponent : public Component
{
protected:
    virtual ~MeshComponent();

public:
    MeshComponent(Mesh* mesh_);

    void SetMesh(Mesh* mesh);
    Mesh* GetMesh() const;

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

private:
    Vector<MeshLODDescriptor> meshLODDescriptors;
    Mesh* mesh = nullptr;

    DAVA_VIRTUAL_REFLECTION(MeshComponent, Component);
};
}
