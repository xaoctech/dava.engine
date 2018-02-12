#pragma once

#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Math/Quaternion.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
class Entity;
class TransformSystem;
class SerializationContext;

class TransformInterpolationComponent : public Component
{
    friend class TransformSystem;

    DAVA_VIRTUAL_REFLECTION(TransformInterpolationComponent, Component);

public:
    float32 time = 1.0f;
    float32 spring = 0.5f;

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void Reset();
    void ApplyImmediately();

private:
    Vector3 startPosition;
    Quaternion startRotation;
    Vector3 startScale;

    Vector3 curPosition;
    Quaternion curRotation;
    Vector3 curScale;

    bool done = false;
    bool immediately = true;
    float32 elapsed = 0.0f;
};
} // namespace DAVA
