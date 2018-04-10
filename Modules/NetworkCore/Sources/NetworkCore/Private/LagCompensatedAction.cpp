#include "NetworkCore/LagCompensatedAction.h"
#include "NetworkCore/SnapshotUtils.h"
#include "NetworkCore/NetworkCoreUtils.h"
#include "NetworkCore/Scene3D/Components/NetworkTransformComponent.h"
#include "NetworkCore/Scene3D/Components/NetworkReplicationComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkGameModeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/NetworkTimeSingleComponent.h"
#include "NetworkCore/Scene3D/Components/SingleComponents/SnapshotSingleComponent.h"

#include <Debug/DVAssert.h>
#include <Logger/Logger.h>
#include <Concurrency/Thread.h>
#include <Entity/ComponentMask.h>
#include <Scene3D/Entity.h>
#include <Scene3D/Scene.h>
#include <Scene3D/Components/TransformComponent.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
namespace LagCompensationDetail
{
// TODO: SaveComponentState/RestoreComponentState is not the best way to save components,
// better use snapshot system for that in the future (making a snapshot for a single entity isn't available yet)

// Component + field values it had before lag compensation
// Used to restore field values
struct ComponentBackup final
{
    const Component* component;
    const Vector<Any> fields;
};

// Save all field values for specific component
Vector<Any> SaveComponentState(const Component& component)
{
    const ReflectedObject reflectedObject(&component);
    const Reflection reflection = Reflection::Create(reflectedObject);
    const Vector<Reflection::Field> fields = reflection.GetFields();

    Vector<Any> fieldsValues;
    fieldsValues.reserve(fields.size());
    for (const Reflection::Field& f : fields)
    {
        if (f.ref.GetMeta<M::ReadOnly>() == nullptr)
        {
            Any value = f.ref.GetValue();
            fieldsValues.push_back(std::move(value));
        }
    }

    return fieldsValues;
}

// Restore component's fields from the backup
void RestoreComponentState(const ComponentBackup& state)
{
    DVASSERT(state.component != nullptr);
    const Component& component = *state.component;

    const ReflectedObject reflectedObject(&component);
    const Reflection reflection = Reflection::Create(reflectedObject);
    const Vector<Reflection::Field> fields = reflection.GetFields();

    // Use separate counter for backup fields vector since we only save non-readonly fields
    uint32 fieldBackupIndex = 0;
    for (uint32 i = 0; i < fields.size(); ++i)
    {
        const Reflection::Field& f = fields[i];
        if (f.ref.GetMeta<M::ReadOnly>() == nullptr)
        {
            f.ref.SetValue(state.fields[fieldBackupIndex]);
            ++fieldBackupIndex;
        }
    }
}

// Rewind component to state from the snapshot
void RewindComponent(Component& component, const NetworkID entityNetworkId, Snapshot* snapshot)
{
    DVASSERT(snapshot != nullptr);

    const Entity* entity = component.GetEntity();
    DVASSERT(entity != nullptr);

    const uint32 componentIndex = entity->GetComponentInnerIndex(&component);
    const SnapshotComponentKey componentKey(ComponentUtils::GetRuntimeId(component.GetType()), componentIndex);
    const bool applyResult = SnapshotUtils::ApplySnapshot(snapshot, entityNetworkId, componentKey, &component);
    DVASSERT(applyResult);
}

void CopyNetworkTransformToTransform(const Entity& e)
{
    const NetworkTransformComponent* networkTransformComponent = e.GetComponent<NetworkTransformComponent>();
    TransformComponent* transformComponent = e.GetComponent<TransformComponent>();
    transformComponent->SetLocalTransform(networkTransformComponent->GetPosition(), networkTransformComponent->GetOrientation(), transformComponent->GetScale());
}
}

void LagCompensatedAction::Invoke(Scene& scene, const NetworkPlayerID& playerId, const uint32 clientFrameId)
{
    // TODO: apply interpolation

    using namespace LagCompensationDetail;

    DVASSERT(Thread::IsMainThread());

    // Use static vectors to avoid dynamic allocations for every call
    // This function can only be used from the main thread and nested calls are not allowed
    static Vector<Entity*> affectedEntities;
    static Vector<ComponentBackup> componentBackups;

    // Check that this is not a nested call
    static bool lagCompensationActive = false;
    DVASSERT(lagCompensationActive == false, "Nested calls to LagCompensatedAction::Invoke are not allowed");

    lagCompensationActive = true;

    SCOPE_EXIT
    {
        affectedEntities.clear();
        componentBackups.clear();

        lagCompensationActive = false;
    };

    // If we're on a client, no need for lag compensation
    if (!IsServer(&scene))
    {
        InvokeWithoutLagCompensation(scene);
        return;
    }

    const NetworkGameModeSingleComponent* gameModeSingleComponent = scene.GetSingleComponent<NetworkGameModeSingleComponent>();
    DVASSERT(gameModeSingleComponent != nullptr);

    const FastName& playerToken = gameModeSingleComponent->GetToken(playerId);
    DVASSERT(playerToken.IsValid());

    const NetworkTimeSingleComponent* timeSingleComponent = scene.GetSingleComponent<NetworkTimeSingleComponent>();
    DVASSERT(timeSingleComponent != nullptr);

    const int32 playerViewDelay = timeSingleComponent->GetClientViewDelay(playerToken, clientFrameId);

    // If we couldn't get player view delay, lag compensation is not possible, invoke without
    if (playerViewDelay < 0)
    {
        Logger::Error("LagCompensatedAction::Invoke: player's view delay is negative, skipping lag compensation. Player's token: %s, view delay: %d", playerToken.c_str(), playerViewDelay);
        InvokeWithoutLagCompensation(scene);
        return;
    }

    // Frame we should revert components to
    const uint32 finalFrameId = clientFrameId - playerViewDelay;

    SnapshotSingleComponent* snapshotSingleComponent = scene.GetSingleComponent<SnapshotSingleComponent>();
    DVASSERT(snapshotSingleComponent != nullptr);

    Snapshot* snapshot = snapshotSingleComponent->GetServerSnapshot(finalFrameId);
    DVASSERT(snapshot != nullptr);

    // Lag compensation only works with replicated entities
    EntityGroup* networkEntities = scene.AquireEntityGroup<NetworkReplicationComponent>();
    DVASSERT(networkEntities);

    for (Entity* e : networkEntities->GetEntities())
    {
        // Decide if we should rewind any components for this entity
        const ComponentMask* componentMask = GetLagCompensatedComponentsForEntity(e);
        if (componentMask != nullptr && componentMask->IsAnySet())
        {
            const NetworkReplicationComponent* replicationComponent = e->GetComponent<NetworkReplicationComponent>();
            DVASSERT(replicationComponent != nullptr);
            const NetworkID networkId = replicationComponent->GetNetworkID();

            // True = at least one component in this entity has been rewound
            bool rewound = false;

            // Backup and rewind required components
            const uint32 numComponents = e->GetComponentCount();
            for (uint32 i = 0; i < numComponents; ++i)
            {
                Component* c = e->GetComponentByIndex(i);
                DVASSERT(c != nullptr);

                const Type* componentType = c->GetType();
                DVASSERT(componentType != nullptr);

                if (componentMask->IsSet(componentType))
                {
                    rewound = true;

                    Vector<Any> componentFields = SaveComponentState(*c);
                    componentBackups.push_back(ComponentBackup{ c, std::move(componentFields) });

                    RewindComponent(*c, networkId, snapshot);

                    // Network component gets special treatment - when we rewind it, we need to copy it's transform into TransformComponent
                    // Other components can require additional logic that needs to be executed after rewinding
                    // For non-NetworkCore components this logic can be put in `OnComponentsInPast` and `OnComponentsInPresent` and should be implement by a game
                    if (componentType->Is<NetworkTransformComponent>())
                    {
                        CopyNetworkTransformToTransform(*e);
                    }
                }
            }

            if (rewound)
            {
                affectedEntities.push_back(e);
            }
        }
    }

    // Notify that all componens have been rewound
    OnComponentsInPast(scene, affectedEntities);

    // Execute the action
    Action(scene);

    // Restore everything we rewound
    for (const ComponentBackup& componentState : componentBackups)
    {
        RestoreComponentState(componentState);

        const Component* c = componentState.component;
        const Type* componentType = c->GetType();
        if (componentType->Is<NetworkTransformComponent>())
        {
            Entity* e = c->GetEntity();
            DVASSERT(e != nullptr);

            CopyNetworkTransformToTransform(*e);
        }
    }

    // Notify that all components are back to present
    OnComponentsInPresent(scene, affectedEntities);
}

void LagCompensatedAction::InvokeWithoutLagCompensation(Scene& scene)
{
    Action(scene);
}

void LagCompensatedAction::OnComponentsInPast(Scene& scene, const Vector<Entity*>& entities)
{
}

void LagCompensatedAction::OnComponentsInPresent(Scene& scene, const Vector<Entity*>& entities)
{
}
}