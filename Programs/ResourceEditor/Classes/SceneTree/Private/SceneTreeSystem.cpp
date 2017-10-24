#include "Classes/SceneTree/Private/SceneTreeSystem.h"

#include "Classes/Commands2/Base/RECommandNotificationObject.h"
#include "Classes/Commands2/ParticleEditorCommands.h"
#include "Classes/Commands2/ParticleEmitterMoveCommands.h"
#include "Classes/Commands2/ParticleForceMoveCommand.h"
#include "Classes/Commands2/ParticleLayerMoveCommand.h"
#include "Classes/Commands2/RECommandIDs.h"

#include "Classes/Qt/Scene/System/EditorParticlesSystem.h"

#include <Command/CommandBatch.h>

namespace SceneTreeSystemDetail
{
class ResetSolidFlagForSave : public DAVA::Command
{
public:
    ResetSolidFlagForSave(DAVA::Entity* e)
        : DAVA::Command("")
        , entity(e)
    {
        prevValue = entity->GetSolid();
    }

    void Redo() override
    {
        entity->SetSolid(false);
    }

    void Undo() override
    {
        entity->SetSolid(prevValue);
    }

private:
    DAVA::Entity* entity = nullptr;
    bool prevValue = false;
};

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
        syncSnapshot.objectsToRefetch[CalcEntityDepth(parentEntity)].push_back(Selectable(DAVA::Any(parentEntity)));
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
    syncSnapshot.changedObjects.emplace(Selectable(DAVA::Any(entity)));
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

    Selectable obj = Selectable(DAVA::Any(entity));

    syncSnapshot.changedObjects.erase(obj);
    syncSnapshot.removedObjects[CalcEntityDepth(entity)].push_back(obj);
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
    syncSnapshot.changedObjects.emplace(Selectable(DAVA::Any(entity)));
}

void SceneTreeSystem::PrepareForRemove()
{
    SyncFinished();
}

void SceneTreeSystem::Process(DAVA::float32 timeElapsed)
{
    if (syncRequested == false && syncSnapshot.IsEmpty() == false)
    {
        syncIsNecessary.Emit();
        syncRequested = true;
    }
}

void SceneTreeSystem::ProcessCommand(const RECommandNotificationObject& commandNotification)
{
    using namespace SceneTreeSystemDetail;
    if (IsSystemEnabled() == false)
    {
        return;
    }

    commandNotification.ForEachWithCast<CommandAddParticleEmitter>(CMDID_PARTICLE_EMITTER_ADD, [&](const CommandAddParticleEmitter* command) {
        static_assert(std::is_base_of<CommandAction, CommandAddParticleEmitter>::value, "You should support undo for this command here");
        DAVA::Entity* entity = command->GetEntity();
        syncSnapshot.objectsToRefetch[CalcEntityDepth(entity)].push_back(Selectable(DAVA::Any(entity)));
    });

    commandNotification.ForEachWithCast<CommandRemoveParticleEmitter>(CMDID_PARTICLE_EFFECT_EMITTER_REMOVE, [&](const CommandRemoveParticleEmitter* command) {
        DAVA::ParticleEffectComponent* component = command->GetEffect();
        DAVA::Entity* entity = component->GetEntity();
        DAVA::ParticleEmitterInstance* emitterInstance = command->GetEmitterInstance();
        if (commandNotification.redo == true)
        {
            syncSnapshot.removedObjects[CalcEntityDepth(entity) + CalcParticleElementsDepth(component, emitterInstance) + 1].push_back(Selectable(DAVA::Any(emitterInstance)));
        }
        else
        {
            syncSnapshot.objectsToRefetch[CalcEntityDepth(entity)].push_back(Selectable(DAVA::Any(entity)));
        }
    });

    commandNotification.ForEachWithCast<CommandUpdateEmitter>(CMDID_PARTICLE_EMITTER_UPDATE, [&](const CommandUpdateEmitter* command) {
        static_assert(std::is_base_of<CommandAction, CommandUpdateEmitter>::value, "You should support undo for this command here");
        DAVA::ParticleEmitterInstance* emitterInstance = command->GetEmitterInstance();
        syncSnapshot.changedObjects.insert(Selectable(DAVA::Any(emitterInstance)));
    });

    commandNotification.ForEachWithCast<ParticleEmitterMoveCommand>(CMDID_PARTICLE_EMITTER_MOVE, [&](const ParticleEmitterMoveCommand* command) {
        DAVA::Entity* oldEntity = command->GetOldComponent()->GetEntity();
        DAVA::Entity* newEntity = command->GetNewComponent()->GetEntity();
        DAVA::ParticleEmitterInstance* instance = command->GetEmitterInstance();

        if (commandNotification.redo == true)
        {
            syncSnapshot.objectsToRefetch[CalcEntityDepth(newEntity)].push_back(Selectable(DAVA::Any(newEntity)));
            syncSnapshot.removedObjects[CalcEntityDepth(oldEntity) + 1].push_back(Selectable(DAVA::Any(instance)));
        }
        else
        {
            syncSnapshot.objectsToRefetch[CalcEntityDepth(oldEntity)].push_back(Selectable(DAVA::Any(oldEntity)));
            syncSnapshot.removedObjects[CalcEntityDepth(newEntity) + 1].push_back(Selectable(DAVA::Any(instance)));
        }
    });

    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    commandNotification.ForEachWithCast<CommandAddParticleEmitterLayer>(CMDID_PARTICLE_EMITTER_LAYER_ADD, [&](const CommandAddParticleEmitterLayer* command) {
        static_assert(std::is_base_of<CommandAction, CommandAddParticleEmitterLayer>::value, "You should support undo for this command here");
        DAVA::ParticleEffectComponent* component = command->GetEffectComponent();
        DAVA::Entity* entity = component->GetEntity();
        DAVA::ParticleEmitterInstance* instance = command->GetParentEmitter();

        syncSnapshot.objectsToRefetch[CalcEntityDepth(entity) + CalcParticleElementsDepth(component, instance) + 1].push_back(Selectable(DAVA::Any(instance)));
    });

    commandNotification.ForEachWithCast<CommandCloneParticleEmitterLayer>(CMDID_PARTICLE_EMITTER_LAYER_CLONE, [&](const CommandCloneParticleEmitterLayer* command) {
        static_assert(std::is_base_of<CommandAction, CommandCloneParticleEmitterLayer>::value, "You should support undo for this command here");
        DAVA::ParticleEmitterInstance* instance = command->GetEmitterInstance();
        DAVA::ParticleEffectComponent* component = GetScene()->GetSystem<EditorParticlesSystem>()->GetEmitterOwner(instance);
        DAVA::Entity* entity = component->GetEntity();

        syncSnapshot.objectsToRefetch[CalcEntityDepth(entity) + CalcParticleElementsDepth(component, instance) + 1].push_back(Selectable(DAVA::Any(instance)));
    });

    commandNotification.ForEachWithCast<ParticleLayerMoveCommand>(CMDID_PARTICLE_LAYER_MOVE, [&](const ParticleLayerMoveCommand* command) {
        DAVA::ParticleLayer* layer = command->GetLayer();
        EditorParticlesSystem* system = GetScene()->GetSystem<EditorParticlesSystem>();
        DAVA::ParticleEmitterInstance* oldEmitter = command->GetOldEmitter();
        DAVA::ParticleEmitterInstance* newEmitter = command->GetNewEmitter();
        DAVA::ParticleEffectComponent* oldComponent = system->GetEmitterOwner(oldEmitter);
        DAVA::ParticleEffectComponent* newComponent = system->GetEmitterOwner(newEmitter);

        if (commandNotification.redo == true)
        {
            syncSnapshot.removedObjects[CalcEntityDepth(oldComponent->GetEntity()) + CalcParticleElementsDepth(oldComponent, oldEmitter) + 2].push_back(Selectable(DAVA::Any(layer)));
            syncSnapshot.objectsToRefetch[CalcEntityDepth(newComponent->GetEntity()) + CalcParticleElementsDepth(newComponent, newEmitter) + 1].push_back(Selectable(DAVA::Any(newEmitter)));
        }
        else
        {
            syncSnapshot.removedObjects[CalcEntityDepth(newComponent->GetEntity()) + CalcParticleElementsDepth(newComponent, newEmitter) + 2].push_back(Selectable(DAVA::Any(layer)));
            syncSnapshot.objectsToRefetch[CalcEntityDepth(oldComponent->GetEntity()) + CalcParticleElementsDepth(oldComponent, oldEmitter) + 1].push_back(Selectable(DAVA::Any(oldEmitter)));
        }
    });

    commandNotification.ForEachWithCast<CommandUpdateParticleLayer>(CMDID_PARTICLE_LAYER_UPDATE, [&](const CommandUpdateParticleLayer* command) {
        static_assert(std::is_base_of<CommandAction, CommandUpdateParticleLayer>::value, "You should support undo for this command here");

        EditorParticlesSystem* system = GetScene()->GetSystem<EditorParticlesSystem>();

        DAVA::ParticleLayer* layer = command->GetLayer();
        DAVA::ParticleEffectComponent* component = system->GetEmitterOwner(system->GetRootEmitterLayerOwner(layer));
        DAVA::ParticleEmitterInstance* deletedEmitter = command->GetDeletedEmitter();
        DAVA::ParticleEmitterInstance* createdEmitter = command->GetCreatedEmitter();

        DAVA::int32 layerDepth = CalcEntityDepth(component->GetEntity()) + CalcParticleElementsDepth(component, layer) + 1;

        syncSnapshot.changedObjects.insert(Selectable(DAVA::Any(layer)));
        if (deletedEmitter != nullptr)
        {
            syncSnapshot.removedObjects[layerDepth + 1].push_back(Selectable(DAVA::Any(deletedEmitter)));
        }

        if (createdEmitter != nullptr)
        {
            syncSnapshot.objectsToRefetch[layerDepth].push_back(Selectable(DAVA::Any(layer)));
        }
    });

    commandNotification.ForEachWithCast<CommandUpdateParticleLayerEnabled>(CMDID_PARTICLE_LAYER_UPDATE_ENABLED, [&](const CommandUpdateParticleLayerEnabled* command) {
        static_assert(std::is_base_of<CommandAction, CommandUpdateParticleLayerEnabled>::value, "You should support undo for this command here");
        syncSnapshot.changedObjects.insert(Selectable(DAVA::Any(command->GetLayer())));
    });

    commandNotification.ForEachWithCast<CommandRemoveParticleEmitterLayer>(CMDID_PARTICLE_EMITTER_LAYER_REMOVE, [&](const CommandRemoveParticleEmitterLayer* command) {
        static_assert(std::is_base_of<CommandAction, CommandRemoveParticleEmitterLayer>::value, "You should support undo for this command here");
        DAVA::ParticleEffectComponent* component = command->GetEffectComponent();
        DAVA::Entity* entity = component->GetEntity();
        DAVA::ParticleEmitterInstance* emitterInstance = command->GetEmitterInstance();

        syncSnapshot.removedObjects[CalcEntityDepth(entity) + CalcParticleElementsDepth(component, emitterInstance) + 2].push_back(Selectable(DAVA::Any(command->GetLayer())));
    });

    //////////////////////////////////////////////////////////////////////////////////////////////////////////

    commandNotification.ForEachWithCast<CommandAddParticleEmitterForce>(CMDID_PARTICLE_EMITTER_FORCE_ADD, [&](const CommandAddParticleEmitterForce* command) {
        static_assert(std::is_base_of<CommandAction, CommandAddParticleEmitterForce>::value, "You should support undo for this command here");
        DAVA::ParticleEffectComponent* component = command->GetEffectComponent();
        DAVA::Entity* entity = component->GetEntity();
        DAVA::ParticleLayer* layer = command->GetLayer();

        syncSnapshot.objectsToRefetch[CalcEntityDepth(entity) + CalcParticleElementsDepth(component, layer) + 1].push_back(Selectable(DAVA::Any(layer)));
    });

    commandNotification.ForEachWithCast<CommandRemoveParticleEmitterForce>(CMDID_PARTICLE_EMITTER_FORCE_REMOVE, [&](const CommandRemoveParticleEmitterForce* command) {
        static_assert(std::is_base_of<CommandAction, CommandRemoveParticleEmitterForce>::value, "You should support undo for this command here");
        DAVA::ParticleEffectComponent* component = command->GetEffectComponent();
        DAVA::Entity* entity = component->GetEntity();
        DAVA::ParticleLayer* layer = command->GetLayer();

        syncSnapshot.removedObjects[CalcEntityDepth(entity) + CalcParticleElementsDepth(component, layer) + 2].push_back(Selectable(DAVA::Any(command->GetForce())));
    });

    commandNotification.ForEachWithCast<ParticleForceMoveCommand>(CMDID_PARTICLE_FORCE_MOVE, [&](const ParticleForceMoveCommand* command) {
        EditorParticlesSystem* system = GetScene()->GetSystem<EditorParticlesSystem>();
        DAVA::ParticleEffectComponent* oldComponent = system->GetRootEmitterLayerOwner(command->oldLayer)->GetOwner();
        DAVA::ParticleEffectComponent* newComponent = system->GetRootEmitterLayerOwner(command->newLayer)->GetOwner();

        if (commandNotification.redo == true)
        {
            syncSnapshot.objectsToRefetch[CalcEntityDepth(newComponent->GetEntity()) + CalcParticleElementsDepth(newComponent, command->newLayer) + 1].push_back(Selectable(DAVA::Any(command->newLayer)));
            syncSnapshot.removedObjects[CalcEntityDepth(oldComponent->GetEntity()) + CalcParticleElementsDepth(oldComponent, command->oldLayer) + 2].push_back(Selectable(DAVA::Any(command->force)));
        }
        else
        {
            syncSnapshot.objectsToRefetch[CalcEntityDepth(oldComponent->GetEntity()) + CalcParticleElementsDepth(oldComponent, command->oldLayer) + 1].push_back(Selectable(DAVA::Any(command->oldLayer)));
            syncSnapshot.removedObjects[CalcEntityDepth(newComponent->GetEntity()) + CalcParticleElementsDepth(newComponent, command->newLayer) + 2].push_back(Selectable(DAVA::Any(command->force)));
        }
    });
}

const SceneTreeSystem::SyncSnapshot& SceneTreeSystem::GetSyncSnapshot() const
{
    return syncSnapshot;
}

void SceneTreeSystem::SyncFinished()
{
    syncSnapshot = SyncSnapshot();
    syncRequested = false;
}

std::unique_ptr<DAVA::Command> SceneTreeSystem::PrepareForSave(bool saveForGame)
{
    DAVA::CommandBatch* batch = new DAVA::CommandBatch();
    DAVA::Scene* scene = GetScene();

    DAVA::Function<void(DAVA::Entity*)> fn = [batch, &fn](DAVA::Entity* e) {
        batch->Add(std::make_unique<SceneTreeSystemDetail::ResetSolidFlagForSave>(e));
        for (DAVA::int32 i = 0; i < e->GetChildrenCount(); ++i)
        {
            fn(e->GetChild(i));
        }
    };

    fn(scene);
    return std::unique_ptr<DAVA::Command>(batch);
}
