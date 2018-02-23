#pragma once

#include "NetworkCore/NetworkTypes.h"
#include "Reflection/Reflection.h"
#include "Debug/DVAssert.h"
#include "Entity/Component.h"
#include "Base/Introspection.h"
#include "Scene3D/Entity.h"
#include "NetworkCore/Private/NetworkSerialization.h"
#include "Scene3D/SceneFile/SerializationContext.h"
#include "Math/Vector.h"

namespace DAVA
{
class NetworkTransformComponent : public Component
{
    DAVA_VIRTUAL_REFLECTION(NetworkTransformComponent, Component);

public:
    NetworkTransformComponent() = default;
    ~NetworkTransformComponent() = default;

    Component* Clone(Entity* toEntity) override;
    void Serialize(KeyedArchive* archive, SerializationContext* serializationContext) override;
    void Deserialize(KeyedArchive* archive, SerializationContext* serializationContext) override;

    void CopyTransformFrom(TransformComponent* copyFrom);

    const Vector3& GetPosition() const;
    void SetPosition(const Vector3& position_);
    const Quaternion& GetOrientation() const;
    void SetOrientation(const Quaternion& orientation_);

public:
    Vector3 position;
    Quaternion orientation;
};
}
