#include "Classes/SceneTree/Private/SceneTreeSystem.h"

#include "Classes/Commands2/RECommandIDs.h"
#include "Classes/Commands2/Base/RECommandNotificationObject.h"
#include "Classes/Commands2/ParticleEditorCommands.h"
#include "Classes/Commands2/ParticleEmitterMoveCommands.h"
#include "Classes/Commands2/ParticleForceMoveCommand.h"
#include "Classes/Commands2/ParticleLayerMoveCommand.h"

namespace SceneTreeSystemDetail
{
DAVA::uint32 CalcEntityDepth(DAVA::Entity* e)
{
    DAVA::uint32 depth = 0;
    while (e != nullptr)
    {
        ++depth;
        e = e->GetParent();
    }

    return depth;
}

DAVA::int32 CalcParticleElementsDepth(DAVA::ParticleEffectComponent* component, DAVA::BaseObject* wantedObject)
{
    const DAVA::ReflectedType* objType = DAVA::TArc::GetValueReflectedType(DAVA::Any(wantedObject));
    bool isForce = objType == DAVA::ReflectedTypeDB::Get<DAVA::ParticleForce>();
    bool isLayer = objType == DAVA::ReflectedTypeDB::Get<DAVA::ParticleLayer>();
    bool isEmitter = objType == DAVA::ReflectedTypeDB::Get<DAVA::ParticleEmitterInstance>();

#if defined(__DAVAENGINE_DEBUG__)
    auto fn = [](bool v) { return v == true ? 1 : 0; };
    DVASSERT((fn(isForce) + fn(isLayer) + fn(isEmitter)) == 1);
#endif

    DAVA::Stack<DAVA::BaseObject*> objectsPath;
    DAVA::Function<bool(DAVA::ParticleLayer*)> lookupLayer = [&](DAVA::ParticleLayer* layer) -> bool {
        if (isLayer == true)
        {
            if (layer == wantedObject)
            {
                return true;
            }
        }

        if (isForce == true)
        {
            for (DAVA::ParticleForce* force : layer->forces)
            {
                if (force == wantedObject)
                {
                    objectsPath.push(force);
                    return true;
                }
            }
        }

        if (layer->type == DAVA::ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
        {
            DVASSERT(layer->innerEmitter != nullptr);
            objectsPath.push(layer->innerEmitter);
            if (isEmitter == true && layer->innerEmitter == wantedObject)
            {
                return true;
            }

            for (DAVA::ParticleLayer* layer : layer->innerEmitter->GetEmitter()->layers)
            {
                objectsPath.push(layer);
                if (lookupLayer(layer) == true)
                {
                    return true;
                }
                objectsPath.pop();
            }
            objectsPath.pop();
        }

        return false;
    };

    for (DAVA::uint32 i = 0; i < component->GetEmittersCount(); ++i)
    {
        DAVA::ParticleEmitterInstance* instance = component->GetEmitterInstance(i);
        objectsPath.push(instance);
        if (instance == wantedObject)
        {
            return static_cast<DAVA::int32>(objectsPath.size());
        }

        for (DAVA::ParticleLayer* layer : instance->GetEmitter()->layers)
        {
            objectsPath.push(layer);
            if (lookupLayer(layer) == true)
            {
                return static_cast<DAVA::int32>(objectsPath.size());
            }
            objectsPath.pop();
        }

        objectsPath.pop();
    }

    DVASSERT(false);
    return 0;
}

} // namespace SceneTreeSystemDetail

SceneTreeSystem::SceneTreeSystem(DAVA::Scene* scene)
    : SceneSystem(scene)
{
}

void SceneTreeSystem::RegisterEntity(DAVA::Entity* entity)
{
    using namespace SceneTreeSystemDetail;
    if (IsSystemEnabled() == false)
    {
        return;
    }

    if (entity == GetScene())
    {
        return;
    }

    DAVA::Entity* parentEntity = entity->GetParent();
    if (parentEntity != nullptr)
    {
        GetNextSyncSnapshot().objectsToRefetch[CalcEntityDepth(parentEntity)].push_back(Selectable(DAVA::Any(parentEntity)));
    }
}

void SceneTreeSystem::RegisterComponent(DAVA::Entity* entity, DAVA::Component* component)
{
    if (IsSystemEnabled() == false)
    {
        return;
    }

    if (entity == GetScene())
    {
        return;
    }
    GetNextSyncSnapshot().changedObjects.emplace(Selectable(DAVA::Any(entity)));
}

void SceneTreeSystem::UnregisterEntity(DAVA::Entity* entity)
{
    using namespace SceneTreeSystemDetail;
    if (IsSystemEnabled() == false)
    {
        return;
    }

    if (entity == GetScene())
    {
        return;
    }
    GetNextSyncSnapshot().removedObjects[CalcEntityDepth(entity)].push_back(Selectable(DAVA::Any(entity)));
}

void SceneTreeSystem::UnregisterComponent(DAVA::Entity* entity, DAVA::Component* component)
{
    if (IsSystemEnabled() == false)
    {
        return;
    }

    if (entity == GetScene())
    {
        return;
    }
    GetNextSyncSnapshot().changedObjects.emplace(Selectable(DAVA::Any(entity)));
}

void SceneTreeSystem::PrepareForRemove()
{
    syncSnapshots[0] = syncSnapshots[1] = SyncSnapshot();
}

void SceneTreeSystem::Process(DAVA::float32 timeElapsed)
{
    // if current snapshot is empty, it means that scene tree has already taken this part of diff
    // if next snapshot is not empty, it means that there is a new changes that should be synced with scene tree
    if (syncSnapshots[GetCurrentSyncSnapshotIndex()].IsEmpty() == true &&
        syncSnapshots[GetNextSyncSnapshotIndex()].IsEmpty() == false)
    {
        MoveToNextSnapshot();
        syncIsNecessary.Emit();
    }
}

void SceneTreeSystem::ProcessCommand(const RECommandNotificationObject& commandNotification)
{
    using namespace SceneTreeSystemDetail;
    if (IsSystemEnabled() == false)
    {
        return;
    }

    auto checkCommandAction = [](const CommandAction* action) {
    };

    commandNotification.ForEachWithCast<CommandAddParticleEmitter>([&](const CommandAddParticleEmitter* command) {
        checkCommandAction(command);
        DAVA::Entity* entity = command->GetEntity();
        GetNextSyncSnapshot().objectsToRefetch[CalcEntityDepth(entity)].push_back(Selectable(DAVA::Any(entity)));
    },
                                                                   CMDID_PARTICLE_EMITTER_ADD);

    commandNotification.ForEachWithCast<CommandRemoveParticleEmitter>([&](const CommandRemoveParticleEmitter* command) {
        DAVA::ParticleEffectComponent* component = command->GetEffect();
        DAVA::Entity* entity = component->GetEntity();
        DAVA::ParticleEmitterInstance* emitterInstance = command->GetEmitterInstance();
        if (commandNotification.redo == true)
        {
            GetNextSyncSnapshot().removedObjects[CalcEntityDepth(entity) + CalcParticleElementsDepth(component, emitterInstance) + 1].push_back(Selectable(DAVA::Any(emitterInstance)));
        }
        else
        {
            GetNextSyncSnapshot().objectsToRefetch[CalcEntityDepth(entity)].push_back(Selectable(DAVA::Any(entity)));
        }
    },
                                                                      CMDID_PARTICLE_EFFECT_EMITTER_REMOVE);

    commandNotification.ForEachWithCast<CommandUpdateEmitter>([&](const CommandUpdateEmitter* command) {
        checkCommandAction(command);
        DAVA::ParticleEmitterInstance* emitterInstance = command->GetEmitterInstance();
        GetNextSyncSnapshot().changedObjects.insert(Selectable(DAVA::Any(emitterInstance)));
    },
                                                              CMDID_PARTICLE_EMITTER_UPDATE);

    commandNotification.ForEachWithCast<ParticleEmitterMoveCommand>([&](const ParticleEmitterMoveCommand* command) {
        DAVA::Entity* oldEntity = command->GetOldComponent()->GetEntity();
        DAVA::Entity* newEntity = command->GetNewComponent()->GetEntity();
        DAVA::ParticleEmitterInstance* instance = command->GetEmitterInstance();

        SyncSnapshot& snapshot = GetNextSyncSnapshot();
        if (commandNotification.redo == true)
        {
            snapshot.objectsToRefetch[CalcEntityDepth(newEntity)].push_back(Selectable(DAVA::Any(newEntity)));
            snapshot.removedObjects[CalcEntityDepth(oldEntity) + 1].push_back(Selectable(DAVA::Any(instance)));
        }
        else
        {
            snapshot.objectsToRefetch[CalcEntityDepth(oldEntity)].push_back(Selectable(DAVA::Any(oldEntity)));
            snapshot.removedObjects[CalcEntityDepth(newEntity) + 1].push_back(Selectable(DAVA::Any(instance)));
        }
    },
                                                                    CMDID_PARTICLE_EMITTER_MOVE);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    commandNotification.ForEachWithCast<CommandAddParticleEmitterLayer>([&](const CommandAddParticleEmitterLayer* command) {
        checkCommandAction(command);
        DAVA::ParticleEffectComponent* component = command->GetEffectComponent();
        DAVA::Entity* entity = component->GetEntity();
        DAVA::ParticleEmitterInstance* instance = command->GetParentEmitter();

        GetNextSyncSnapshot().objectsToRefetch[CalcEntityDepth(entity) + CalcParticleElementsDepth(component, instance) + 1].push_back(Selectable(DAVA::Any(instance)));
    },
                                                                        CMDID_PARTICLE_EMITTER_LAYER_ADD);

    commandNotification.ForEachWithCast<CommandCloneParticleEmitterLayer>([&](const CommandCloneParticleEmitterLayer* command) {
        checkCommandAction(command);
        DAVA::ParticleEmitterInstance* instance = command->GetEmitterInstance();
        DAVA::ParticleEffectComponent* component = GetScene()->GetSystem<EditorParticlesSystem>()->GetEmitterOwner(instance);
        DAVA::Entity* entity = component->GetEntity();

        GetNextSyncSnapshot().objectsToRefetch[CalcEntityDepth(entity) + CalcParticleElementsDepth(component, instance) + 1].push_back(Selectable(DAVA::Any(instance)));
    },
                                                                          CMDID_PARTICLE_EMITTER_LAYER_CLONE);

    commandNotification.ForEachWithCast<ParticleLayerMoveCommand>([&](const ParticleLayerMoveCommand* command) {
        DAVA::ParticleLayer* layer = command->GetLayer();
        EditorParticlesSystem* system = GetScene()->GetSystem<EditorParticlesSystem>();
        DAVA::ParticleEmitterInstance* oldEmitter = command->GetOldEmitter();
        DAVA::ParticleEmitterInstance* newEmitter = command->GetNewEmitter();
        DAVA::ParticleEffectComponent* oldComponent = system->GetEmitterOwner(oldEmitter);
        DAVA::ParticleEffectComponent* newComponent = system->GetEmitterOwner(newEmitter);

        SyncSnapshot& snapshot = GetNextSyncSnapshot();
        if (commandNotification.redo == true)
        {
            snapshot.removedObjects[CalcEntityDepth(oldComponent->GetEntity()) + CalcParticleElementsDepth(oldComponent, oldEmitter) + 2].push_back(Selectable(DAVA::Any(layer)));
            snapshot.objectsToRefetch[CalcEntityDepth(newComponent->GetEntity()) + CalcParticleElementsDepth(newComponent, newEmitter) + 1].push_back(Selectable(DAVA::Any(newEmitter)));
        }
        else
        {
            snapshot.removedObjects[CalcEntityDepth(newComponent->GetEntity()) + CalcParticleElementsDepth(newComponent, newEmitter) + 2].push_back(Selectable(DAVA::Any(layer)));
            snapshot.objectsToRefetch[CalcEntityDepth(oldComponent->GetEntity()) + CalcParticleElementsDepth(oldComponent, oldEmitter) + 1].push_back(Selectable(DAVA::Any(oldEmitter)));
        }

    },
                                                                  CMDID_PARTICLE_LAYER_MOVE);

    commandNotification.ForEachWithCast<CommandUpdateParticleLayer>([&](const CommandUpdateParticleLayer* command) {
        checkCommandAction(command);

        EditorParticlesSystem* system = GetScene()->GetSystem<EditorParticlesSystem>();

        DAVA::ParticleLayer* layer = command->GetLayer();
        DAVA::ParticleEffectComponent* component = system->GetEmitterOwner(system->GetRootEmitterLayerOwner(layer));
        DAVA::ParticleEmitterInstance* deletedEmitter = command->GetDeletedEmitter();
        DAVA::ParticleEmitterInstance* createdEmitter = command->GetCreatedEmitter();

        DAVA::int32 layerDepth = CalcEntityDepth(component->GetEntity()) + CalcParticleElementsDepth(component, layer) + 1;

        SyncSnapshot& snapshot = GetNextSyncSnapshot();
        snapshot.changedObjects.insert(Selectable(DAVA::Any(layer)));
        if (deletedEmitter != nullptr)
        {
            snapshot.removedObjects[layerDepth + 1].push_back(Selectable(DAVA::Any(deletedEmitter)));
        }

        if (createdEmitter != nullptr)
        {
            snapshot.objectsToRefetch[layerDepth].push_back(Selectable(DAVA::Any(layer)));
        }
    },
                                                                    CMDID_PARTICLE_LAYER_UPDATE);

    commandNotification.ForEachWithCast<CommandUpdateParticleLayerEnabled>([&](const CommandUpdateParticleLayerEnabled* command) {
        checkCommandAction(command);
        GetNextSyncSnapshot().changedObjects.insert(Selectable(DAVA::Any(command->GetLayer())));
    },
                                                                           CMDID_PARTICLE_LAYER_UPDATE_ENABLED);

    commandNotification.ForEachWithCast<CommandRemoveParticleEmitterLayer>([&](const CommandRemoveParticleEmitterLayer* command) {
        checkCommandAction(command);
        DAVA::ParticleEffectComponent* component = command->GetEffectComponent();
        DAVA::Entity* entity = component->GetEntity();
        DAVA::ParticleEmitterInstance* emitterInstance = command->GetEmitterInstance();

        GetNextSyncSnapshot().removedObjects[CalcEntityDepth(entity) + CalcParticleElementsDepth(component, emitterInstance) + 2].push_back(Selectable(DAVA::Any(command->GetLayer())));
    },
                                                                           CMDID_PARTICLE_EMITTER_LAYER_REMOVE);

    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    commandNotification.ForEachWithCast<CommandAddParticleEmitterForce>([&](const CommandAddParticleEmitterForce* command) {
        checkCommandAction(command);
        DAVA::ParticleEffectComponent* component = command->GetEffectComponent();
        DAVA::Entity* entity = component->GetEntity();
        DAVA::ParticleLayer* layer = command->GetLayer();

        GetNextSyncSnapshot().objectsToRefetch[CalcEntityDepth(entity) + CalcParticleElementsDepth(component, layer) + 1].push_back(Selectable(DAVA::Any(layer)));
    },
                                                                        CMDID_PARTICLE_EMITTER_FORCE_ADD);

    commandNotification.ForEachWithCast<CommandRemoveParticleEmitterForce>([&](const CommandRemoveParticleEmitterForce* command) {
        checkCommandAction(command);
        DAVA::ParticleEffectComponent* component = command->GetEffectComponent();
        DAVA::Entity* entity = component->GetEntity();
        DAVA::ParticleLayer* layer = command->GetLayer();

        GetNextSyncSnapshot().removedObjects[CalcEntityDepth(entity) + CalcParticleElementsDepth(component, layer) + 2].push_back(Selectable(DAVA::Any(command->GetForce())));
    },
                                                                           CMDID_PARTICLE_EMITTER_FORCE_REMOVE);

    commandNotification.ForEachWithCast<ParticleForceMoveCommand>([&](const ParticleForceMoveCommand* command) {
        EditorParticlesSystem* system = GetScene()->GetSystem<EditorParticlesSystem>();
        DAVA::ParticleEffectComponent* oldComponent = system->GetRootEmitterLayerOwner(command->oldLayer)->GetOwner();
        DAVA::ParticleEffectComponent* newComponent = system->GetRootEmitterLayerOwner(command->newLayer)->GetOwner();

        SyncSnapshot& snapshot = GetNextSyncSnapshot();
        if (commandNotification.redo == true)
        {
            snapshot.objectsToRefetch[CalcEntityDepth(newComponent->GetEntity()) + CalcParticleElementsDepth(newComponent, command->newLayer) + 1].push_back(Selectable(DAVA::Any(command->newLayer)));
            snapshot.removedObjects[CalcEntityDepth(oldComponent->GetEntity()) + CalcParticleElementsDepth(oldComponent, command->oldLayer) + 2].push_back(Selectable(DAVA::Any(command->force)));
        }
        else
        {
            snapshot.objectsToRefetch[CalcEntityDepth(oldComponent->GetEntity()) + CalcParticleElementsDepth(oldComponent, command->oldLayer) + 1].push_back(Selectable(DAVA::Any(command->oldLayer)));
            snapshot.removedObjects[CalcEntityDepth(newComponent->GetEntity()) + CalcParticleElementsDepth(newComponent, command->newLayer) + 2].push_back(Selectable(DAVA::Any(command->force)));
        }
    },
                                                                  CMDID_PARTICLE_FORCE_MOVE);
}

const SceneTreeSystem::SyncSnapshot& SceneTreeSystem::GetSyncSnapshot() const
{
    return syncSnapshots[GetCurrentSyncSnapshotIndex()];
}

void SceneTreeSystem::SyncFinished()
{
    syncSnapshots[GetCurrentSyncSnapshotIndex()] = SyncSnapshot();
}

SceneTreeSystem::SyncSnapshot& SceneTreeSystem::GetNextSyncSnapshot()
{
    return syncSnapshots[GetNextSyncSnapshotIndex()];
}
