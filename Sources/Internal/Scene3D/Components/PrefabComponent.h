#pragma once

#include "Base/BaseTypes.h"
#include "Reflection/Reflection.h"
#include "Entity/Component.h"
#include "Scene3D/Entity.h"

namespace DAVA
{
class SerializationContext;
class PrefabComponent : public Component
{
protected:
    virtual ~PrefabComponent();

public:
    PrefabComponent();

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

protected:
    FilePath prefabFilepath;
    bool reference = true;
    DAVA_VIRTUAL_REFLECTION(PrefabComponent, Component);
};
}
