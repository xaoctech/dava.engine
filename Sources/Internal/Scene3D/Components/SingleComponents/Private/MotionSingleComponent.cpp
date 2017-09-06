#include "Scene3D/Entity.h"
#include "Scene3D/Components/ComponentHelpers.h"
#include "Scene3D/Components/SingleComponents/MotionSingleComponent.h"
#include "Utils/Utils.h"

namespace DAVA
{
void MotionSingleComponent::Clear()
{
    animationPhaseEnd.clear();
    rebindAnimation.clear();
    reloadConfig.clear();
}

void MotionSingleComponent::EntityRemoved(const Entity* entity)
{
    MotionComponent* component = GetMotionComponent(entity);
    if (component)
    {
        for (int32 i = int32(animationPhaseEnd.size()) - 1; i >= 0; --i)
        {
            if (animationPhaseEnd[i].first == component)
            {
                animationPhaseEnd[i] = animationPhaseEnd.back();
                animationPhaseEnd.pop_back();
            }
        }

        FindAndRemoveExchangingWithLast(rebindAnimation, component);
        FindAndRemoveExchangingWithLast(reloadConfig, component);
    }
}
}