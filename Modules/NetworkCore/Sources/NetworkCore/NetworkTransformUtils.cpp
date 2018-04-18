#include "NetworkCore/NetworkTransformUtils.h"
#include "NetworkCore/Scene3D/Components/NetworkTransformComponent.h"

#include <Math/Vector.h>
#include <Math/Quaternion.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Components/TransformComponent.h>

namespace DAVA
{
void NetworkTransformUtils::CopyToTransform(const NetworkTransformComponent* networkTransformComponent)
{
    DVASSERT(networkTransformComponent != nullptr);

    Entity* entity = networkTransformComponent->GetEntity();
    DVASSERT(entity != nullptr);

    TransformComponent* transformComponent = entity->GetComponent<TransformComponent>();
    DVASSERT(transformComponent != nullptr);

    const Transform& transform = transformComponent->GetLocalTransform();

    const Vector3& networkPosition = networkTransformComponent->GetPosition();
    const Quaternion& networkOrientation = networkTransformComponent->GetOrientation();

    if (transform.GetTranslation() != networkPosition ||
        transform.GetRotation() != networkOrientation)
    {
        transformComponent->SetLocalTransform(Transform(
                networkPosition, transform.GetScale(), networkOrientation));
    }
}
}