#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/SingleComponents/MotionSingleComponent.h"
#include "Utils/Utils.h"

namespace DAVA
{
void MotionSingleComponent::Clear()
{
    animationEnd.clear();
    rebindSkeleton.clear();
    reloadMotion.clear();

    startSimpleMotion.clear();
    stopSimpleMotion.clear();
    simpleMotionFinished.clear();
}

void MotionSingleComponent::EntityRemoved(const Entity* entity)
{
    MotionComponent* component = GetMotionComponent(entity);
    if (component)
    {
        FindAndRemoveExchangingWithLast(rebindSkeleton, component);
        FindAndRemoveExchangingWithLast(reloadMotion, component);
        FindAndRemoveExchangingWithLast(startSimpleMotion, component);
        FindAndRemoveExchangingWithLast(stopSimpleMotion, component);
        FindAndRemoveExchangingWithLast(simpleMotionFinished, component);
    }
}
}