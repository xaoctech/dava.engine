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

class TransformComponent : public Component
{
    friend class Entity; // <-- huge hack, to be able to call SetParent() function. Should be removed!!!
    friend class TransformSystem;

    DAVA_VIRTUAL_REFLECTION(TransformComponent, Component);

public:
    Matrix4 GetLocalTransform() const;
    const Vector3& GetPosition() const;
    const Quaternion& GetRotation() const;
    const Vector3& GetScale() const;

    const Matrix4& GetWorldTransform() const;
    const Matrix4* GetWorldTransformPtr() const;

    void SetLocalTransform(const Vector3& position, const Quaternion& rotation, const Vector3& scale);
    void SetLocalTransform(const Matrix4& transform);

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

protected:
    void SetParent(Entity* node);
    void SetWorldTransform(const Matrix4& transform);

    void BeginInterpolation();
    void ApplyLocalTransfomChanged();
    void ApplyWorldTransfomChanged();
    void ApplyParentChanged();

private:
    Vector3 position;
    Quaternion rotation;
    Vector3 scale = Vector3(1.0f, 1.0f, 1.0f);

    Matrix4 worldMatrix = Matrix4::IDENTITY;
    const Matrix4* parentMatrix = nullptr;
    Entity* parent = nullptr;
};

inline const Matrix4& TransformComponent::GetWorldTransform() const
{
    return worldMatrix;
}

inline const Matrix4* TransformComponent::GetWorldTransformPtr() const
{
    return &worldMatrix;
}

inline const Vector3& TransformComponent::GetPosition() const
{
    return position;
}

inline const Quaternion& TransformComponent::GetRotation() const
{
    return rotation;
}

inline const Vector3& TransformComponent::GetScale() const
{
    return scale;
}

inline Matrix4 TransformComponent::GetLocalTransform() const
{
    return Matrix4(position, rotation, scale);
}
}
