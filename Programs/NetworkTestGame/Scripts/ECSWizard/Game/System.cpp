#include "TEMPLATESystem.h"

#include <Reflection/ReflectionRegistrator.h>
#include "Scene3D/Scene.h"
#include "Scene3D/Entity.h"
#include "Logger/Logger.h"

using namespace DAVA;

DAVA_VIRTUAL_REFLECTION_IMPL(TEMPLATESystem)
{
    ReflectionRegistrator<TEMPLATESystem>::Begin()[M::Tags("")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &TEMPLATESystem::ProcessFixed)[M::SystemProcess(SP::Group::GAMEPLAY_BEGIN, SP::Type::FIXED, 0.0f)]
    .End();
}

namespace TEMPLATESystemDetail
{
}

TEMPLATESystem::TEMPLATESystem(Scene* scene)
    : SceneSystem(scene, 0)
{
}

void TEMPLATESystem::ProcessFixed(float32 timeElapsed)
{
}
