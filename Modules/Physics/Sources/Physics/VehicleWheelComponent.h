#pragma once

#include <Entity/Component.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class Entity;

class VehicleWheelComponent final : public Component
{
public:
    float32 GetRadius() const;
    void SetRadius(float32 value);

    float32 GetWidth() const;
    void SetWidth(float32 value);

private:
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    virtual uint32 GetType() const override;
    virtual Component* Clone(Entity* toEntity) override;

    DAVA_VIRTUAL_REFLECTION(VehicleWheelComponent, Component);

private:
    float32 radius = 0.5f;
    float32 width = 0.4f;
};
}