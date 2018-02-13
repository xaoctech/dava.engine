#include "NetworkPhysics/NetworkPhysicsModule.h"
#include "NetworkPhysics/NetworkDynamicBodyComponent.h"
#include "NetworkPhysics/NetworkVehicleCarComponent.h"
#include "NetworkPhysics/CharacterMirrorsSingleComponent.h"
#include "NetworkPhysics/NetworkPhysicsSystem.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
NetworkPhysicsModule::NetworkPhysicsModule(Engine* engine)
    : IModule(engine)
{
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkPhysicsModule);

    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkPhysicsSystem);

    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkDynamicBodyComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkVehicleCarComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(CharacterMirrorsSingleComponent);
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
