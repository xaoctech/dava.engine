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

enum class InterpolationState : uint8
{
    DISABLED = 0,
    FIXED,
    ELASTIC,
    TRANSIENT
};

class TransformInterpolationComponent : public Component
{
    friend class TransformSystem;

    DAVA_VIRTUAL_REFLECTION(TransformInterpolationComponent, Component);

public:
    float32 time = 1.0f;
    float32 spring = 0.5f;

    InterpolationState state = InterpolationState::ELASTIC;

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void ApplyImmediately();
    void SetNewTransform(const Vector3& position, const Quaternion& rotation, const Vector3& scale);

private:
    void Reset();

    Vector3 prevPosition;
    Quaternion prevRotation;
    Vector3 prevScale;

    Vector3 curPosition;
    Quaternion curRotation;
    Vector3 curScale;

    bool done = false;
    bool immediately = true;
    float32 elapsed = 0.0f;

    bool isInit = false;
};
} // namespace DAVA
