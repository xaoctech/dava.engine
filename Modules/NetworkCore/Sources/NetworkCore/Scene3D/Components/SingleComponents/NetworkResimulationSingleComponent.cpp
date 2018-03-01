#include "NetworkResimulationSingleComponent.h"

#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(NetworkResimulationSingleComponent)
{
    ReflectionRegistrator<NetworkResimulationSingleComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

void NetworkResimulationSingleComponent::SetResimulationFrameId(uint32 frameId)
{
    resimulationFrameId = frameId;
}

uint32 NetworkResimulationSingleComponent::GetResimulationFrameId() const
{
    return resimulationFrameId;
}

void NetworkResimulationSingleComponent::Clear()
{
}
} // namespace DAVA