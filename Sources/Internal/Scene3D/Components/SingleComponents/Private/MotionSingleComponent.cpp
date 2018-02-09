#include "Scene3D/Components/SingleComponents/MotionSingleComponent.h"

#include "Reflection/ReflectionRegistrator.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Utils/Utils.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(MotionSingleComponent)
{
    ReflectionRegistrator<MotionSingleComponent>::Begin()
    .ConstructorByPointer()
    .End();
}

void MotionSingleComponent::Clear()
{
    startSimpleMotion.clear();
    stopSimpleMotion.clear();
    simpleMotionFinished.clear();

    rebindSkeleton.clear();
    reloadDescriptor.clear();

    animationEnd.clear();
    animationMarkerReached.clear();
}

void MotionSingleComponent::EntityRemoved(const Entity* entity)
{
    MotionComponent* component = GetMotionComponent(entity);
    if (component)
    {
        FindAndRemoveExchangingWithLast(rebindSkeleton, component);
        FindAndRemoveExchangingWithLast(reloadDescriptor, component);
        FindAndRemoveExchangingWithLast(startSimpleMotion, component);
        FindAndRemoveExchangingWithLast(stopSimpleMotion, component);
        FindAndRemoveExchangingWithLast(simpleMotionFinished, component);
    }
}
}