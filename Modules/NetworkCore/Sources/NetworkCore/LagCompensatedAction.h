#pragma once

#include "NetworkCore/NetworkTypes.h"

namespace DAVA
{
class Scene;
class Entity;
class Component;
struct ComponentMask;

/**
    Lag compensated action is an action that gets executed from specific player's perspective.
    That is, prior to executing the action, objects are rewound to be in the same state as player saw them while doing the action on a client.

    This class is responsible for rewinding components back in time, executing an action, and finally reverting all components to present state.

    Derived classes should override virtual methods of this class which will be called when needed during `Invoke` call.
*/
class LagCompensatedAction
{
public:
    virtual ~LagCompensatedAction() = default;

    /**
        Invoke the action.
        If this method executes on a server, lag compensation is applied, unless we couldn't get player's view delay information.
    */
    void Invoke(Scene& scene, const NetworkPlayerID& playerId, const uint32 clientFrameId);

    /**
        Invoke the action without any lag compensation at all.
        Can be useful if lag compensation is an option that can be turned on/off, but the same object can be used to invoke the action, for example:

        ```
        LagCompensatedShootAction shootAction;
        if (lagCompensationOptionEnabled)
        {
            shootAction.Invoke(...);
        }
        else
        {
            shootAction.InvokeWithoutLagCompensation(...);
        }

        Entity* hitPlayer = shootAction.GetHitPlayer();
        
        // Deal damage or whatever
        ...
        ```
    */
    void InvokeWithoutLagCompensation(Scene& scene);

private:
    /**
        Return component mask with bits set for components that need to be lag compensated for specific entity `e`.
        If nullptr or empty mask is returned, entity will not be lag compensated.
    */
    virtual const ComponentMask* GetLagCompensatedComponentsForEntity(Entity* e) = 0;

    /** Invoke the action. */
    virtual void Action(Scene& scene) = 0;

    /**
        Called after all components have been rewound.
        This method can contain additional logic which is needed for entities with rewound components to function properly.

        \param entities Vector of entities whose components have been rewound.
    */
    virtual void OnComponentsInPast(Scene& scene, const Vector<Entity*>& entities);

    /**
        Called after all components have been reverted back to present.
        This method can contain additional logic which is needed for entities with rewound components to function properly.

        \param entities Vector of entities whose components have been rewound.
    */
    virtual void OnComponentsInPresent(Scene& scene, const Vector<Entity*>& entities);
};
}