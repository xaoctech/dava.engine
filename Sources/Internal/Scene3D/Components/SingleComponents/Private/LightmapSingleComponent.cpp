#include "Scene3D/Components/SingleComponents/LightmapSingleComponent.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Utils/Utils.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(LightmapSingleComponent)
{
    ReflectionRegistrator<LightmapSingleComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

void LightmapSingleComponent::Clear()
{
    shadowReceiverParamChanged.clear();
    shadowCasterParamChanged.clear();
    lightmapSizeChanged.clear();
}

void LightmapSingleComponent::EraseComponent(LightmapComponent* component)
{
    FindAndRemoveExchangingWithLast(shadowReceiverParamChanged, component);
    FindAndRemoveExchangingWithLast(shadowCasterParamChanged, component);
    FindAndRemoveExchangingWithLast(lightmapSizeChanged, component);
}
}