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
class NetworkDebugDrawComponent : public Component
{
public:
    NetworkDebugDrawComponent();

    Component* Clone(Entity* toEntity) override;
    ~NetworkDebugDrawComponent();

public:
    DAVA_VIRTUAL_REFLECTION(NetworkDebugDrawComponent, Component);
    AABBox3 box;
};
} //namespace DAVA