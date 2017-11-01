#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/SingleComponents/MotionSingleComponent.h"
#include "Utils/Utils.h"

namespace DAVA
{
void MotionSingleComponent::Clear()
{
    animationEnd.clear();
    rebindAnimation.clear();
    reloadConfig.clear();
}

void MotionSingleComponent::EntityRemoved(const Entity* entity)
{
    MotionComponent* component = GetMotionComponent(entity);
    if (component)
    {
        FindAndRemoveExchangingWithLast(rebindAnimation, component);
        FindAndRemoveExchangingWithLast(reloadConfig, component);
    }
}
}