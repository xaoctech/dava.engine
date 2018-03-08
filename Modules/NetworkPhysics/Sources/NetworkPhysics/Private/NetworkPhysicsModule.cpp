#include "NetworkPhysics/NetworkPhysicsModule.h"
#include "NetworkPhysics/CharacterMirrorsSingleComponent.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
NetworkPhysicsModule::NetworkPhysicsModule(Engine* engine)
    : IModule(engine)
{
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(NetworkPhysicsModule);
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
