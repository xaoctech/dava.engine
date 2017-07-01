#pragma once

#include "Physics/CollisionShapeComponent.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
class HeightFieldShapeComponent : public CollisionShapeComponent
{
public:
    IMPLEMENT_COMPONENT_TYPE(HEIGHT_FIELD_SHAPE_COMPONENT);
    Component* Clone(Entity* toEntity) override;

    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

protected:
#if defined(__DAVAENGINE_DEBUG__)
    void CheckShapeType() const override;
#endif

    void ReleasePxShape() override;

private:
    DAVA_VIRTUAL_REFLECTION(HeightFieldShapeComponent, CollisionShapeComponent);
};
} // namespace DAVA