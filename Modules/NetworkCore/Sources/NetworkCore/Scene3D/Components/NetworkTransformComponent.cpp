#include "NetworkCore/Scene3D/Components/NetworkTransformComponent.h"

#include "NetworkCore/Scene3D/Systems/NetworkTimeSystem.h"

#include <Reflection/ReflectedMeta.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Components/TransformComponent.h>

namespace DAVA
{
REGISTER_CLASS(NetworkTransformComponent);
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkTransformComponent)
{
    ReflectionRegistrator<NetworkTransformComponent>::Begin()[M::CantBeCreatedManualyComponent(), M::Replicable(M::Privacy::PUBLIC)]
    .ConstructorByPointer()
    .Field("Position", &NetworkTransformComponent::position)[M::Replicable(), M::FloatQuantizeParam(8000.0f, 0.005f), M::ComparePrecision(0.01f)]
    .Field("Orientation", &NetworkTransformComponent::orientation)[M::Replicable(), M::QuaternionQuantizeParam(0.0001f), M::ComparePrecision(0.002f)]
    .End();
}

Component* NetworkTransformComponent::Clone(Entity* toEntity)
{
    NetworkTransformComponent* uc = new NetworkTransformComponent();
    uc->SetEntity(toEntity);
    return uc;
}

const Vector3& NetworkTransformComponent::GetPosition() const
{
    return position;
}

void NetworkTransformComponent::SetPosition(const Vector3& position_)
{
    position = position_;
}

const Quaternion& NetworkTransformComponent::GetOrientation() const
{
    return orientation;
}

void NetworkTransformComponent::SetOrientation(const Quaternion& orientation_)
{
    orientation = orientation_;
}

void NetworkTransformComponent::Serialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Serialize(archive, serializationContext);
}

void NetworkTransformComponent::Deserialize(KeyedArchive* archive, SerializationContext* serializationContext)
{
    Component::Deserialize(archive, serializationContext);
}

void NetworkTransformComponent::CopyTransformFrom(TransformComponent* copyFrom)
{
    SetPosition(copyFrom->GetPosition());
    SetOrientation(copyFrom->GetRotation());
}
}
