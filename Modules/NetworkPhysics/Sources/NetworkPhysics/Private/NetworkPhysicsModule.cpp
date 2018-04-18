#include "NetworkPhysics/NetworkPhysicsModule.h"
#include "NetworkPhysics/HitboxesDebugDrawComponent.h"
#include "NetworkPhysics/HitboxesDebugDrawSystem.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
NetworkPhysicsModule::NetworkPhysicsModule(Engine* engine)
    : IModule(engine)
{
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkPhysicsModule);

    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(HitboxesDebugDrawComponent);

    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(HitboxesDebugDrawSystem);
}

void NetworkPhysicsModule::Init()
{
}

void NetworkPhysicsModule::Shutdown()
{
}

DAVA_VIRTUAL_REFLECTION_IMPL(NetworkPhysicsModule)
{
    ReflectionRegistrator<NetworkPhysicsModule>::Begin()
    .End();
}
}
