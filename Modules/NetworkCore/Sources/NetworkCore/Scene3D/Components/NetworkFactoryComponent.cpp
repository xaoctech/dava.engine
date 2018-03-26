#include "NetworkCore/Scene3D/Components/NetworkFactoryComponent.h"

#include "Scene3D/Entity.h"
#include "Scene3D/Scene.h"

#include "Reflection/ReflectionRegistrator.h"
#include "Reflection/ReflectedMeta.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkFactoryComponent)
{
    ReflectionRegistrator<NetworkFactoryComponent>::Begin()[M::Replicable(M::Privacy::PUBLIC)]
    .ConstructorByPointer()
    .Field("Name", &NetworkFactoryComponent::name)[M::Replicable()]
    .End();
}

Component* NetworkFactoryComponent::Clone(Entity* toEntity)
{
    NetworkFactoryComponent* newComp = new NetworkFactoryComponent();
    newComp->SetEntity(toEntity);
    return newComp;
}

void NetworkFactoryComponent::SetInitialTransform(const Vector3& position, const Quaternion& rotation, float32 scale)
{
    initialTransformPtr.reset(new InitialTransform{ position, rotation, scale });
}

UnorderedMap<FastName, NetworkFactoryComponent::ComponentField> NetworkFactoryComponent::componentFieldsCache = {};
const NetworkFactoryComponent::ComponentField& NetworkFactoryComponent::ParsePath(const String& path)
{
    FastName key(path);
    auto findIt = componentFieldsCache.find(key);
    if (findIt == componentFieldsCache.end())
    {
        Vector<String> parts;
        Split(path, "/", parts);
        DVASSERT(parts.size() == 2, "Required format: Component/Filed");

        const String& compName(parts[0]);
        FastName fieldName(parts[1]);
        const ReflectedType* reflectedType = ReflectedTypeDB::GetByPermanentName(compName);
        const Type* type = reflectedType->GetType();
        findIt = componentFieldsCache.emplace(key, ComponentField{ type, fieldName }).first;
    }

    return findIt->second;
}
};
