#include "Scene3D/Components/SingleComponents/MotionSingleComponent.h"
#include "Utils/Utils.h"

namespace DAVA
{
void MotionSingleComponent::Clear()
{
    eventTriggered.clear();
    startAnimation.clear();
    stopAnimation.clear();
    rebindAnimation.clear();
}

void MotionSingleComponent::EntityRemoved(const Entity* entity)
{
    MotionComponent* component = GetMotionComponent(entity);
    if (component)
    {
        for (int32 i = int32(eventTriggered.size()) - 1; i >= 0; --i)
        {
            if (eventTriggered[i].first == component)
            {
                eventTriggered[i] = eventTriggered.back();
                eventTriggered.pop_back();
            }
        }

        FindAndRemoveExchangingWithLast(startAnimation, component);
        FindAndRemoveExchangingWithLast(stopAnimation, component);
        FindAndRemoveExchangingWithLast(rebindAnimation, component);
    }
}
}