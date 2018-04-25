#include "TEMPLATESystem.h"

#include <Debug/ProfilerCPU.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Scene3D/Scene.h>
#include <Reflection/ReflectedMeta.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(TEMPLATESystem)
{
    ReflectionRegistrator<TEMPLATESystem>::Begin()[M::SystemTags("network")]
    .ConstructorByPointer<Scene*>()
    .Method("ProcessFixed", &TEMPLATESystem::ProcessFixed)[M::SystemProcessInfo(SPI::Group::EngineEnd, SPI::Type::Fixed, 0.0f)]
    .End();
}

TEMPLATESystem::TEMPLATESystem(Scene* scene)
    : SceneSystem(scene, 0)
{
}

void TEMPLATESystem::ProcessFixed(float32 timeElapsed)
{
    DAVA_PROFILER_CPU_SCOPE("TEMPLATESystem::ProcessFixed");
}

} //namespace DAVA
