#include "Scene/SceneEditor2.h"
#include "Scene/System/EditorParticlesSystem.h"
#include "Scene/System/CollisionSystem.h"
#include "Scene/System/HoodSystem.h"
#include "Scene/SceneSignals.h"
#include "Main/mainwindow.h"

#include "Classes/Selection/Selection.h"

// framework
#include "Base/BaseTypes.h"
#include "Entity/Component.h"
#include "Particles/ParticleEmitter.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Math/Vector.h"

// particles-related commands
#include "Commands2/ParticleEditorCommands.h"
#include "Commands2/ParticleLayerCommands.h"
#include "Commands2/Base/RECommandNotificationObject.h"

EditorParticlesSystem::EditorParticlesSystem(DAVA::Scene* scene)
    : DAVA::SceneSystem(scene)
{
}

void EditorParticlesSystem::DrawDebugInfoForEffect(DAVA::Entity* effectEntity)
{
    DVASSERT(effectEntity != nullptr);

    SceneCollisionSystem* collisionSystem = static_cast<SceneEditor2*>(GetScene())->collisionSystem;

    DAVA::AABBox3 worldBox;
    DAVA::AABBox3 collBox = collisionSystem->GetBoundingBox(effectEntity);
    DVASSERT(!collBox.IsEmpty());
    collBox.GetTransformedBox(effectEntity->GetWorldTransform(), worldBox);
    DAVA::float32 radius = (collBox.max - collBox.min).Length() / 3;
    GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawIcosahedron(worldBox.GetCenter(), radius, DAVA::Color(0.9f, 0.9f, 0.9f, 0.35f), DAVA::RenderHelper::DRAW_SOLID_DEPTH);
}

void EditorParticlesSystem::DrawEmitter(DAVA::ParticleEmitterInstance* emitter, DAVA::Entity* owner, bool selected)
{
    DVASSERT((emitter != nullptr) && (owner != nullptr));

    SceneCollisionSystem* collisionSystem = ((SceneEditor2*)GetScene())->collisionSystem;

    DAVA::Vector3 center = emitter->GetSpawnPosition();
    TransformPerserveLength(center, DAVA::Matrix3(owner->GetWorldTransform()));
    center += owner->GetWorldTransform().GetTranslationVector();

    DAVA::AABBox3 boundingBox = collisionSystem->GetBoundingBox(owner);
    DVASSERT(!boundingBox.IsEmpty());
    DAVA::float32 radius = (boundingBox.max - boundingBox.min).Length() / 3;
    GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawIcosahedron(center, radius, DAVA::Color(1.0f, 1.0f, 1.0f, 0.5f), DAVA::RenderHelper::DRAW_SOLID_DEPTH);

    if (selected)
    {
        DrawVectorArrow(emitter, center);

        switch (emitter->GetEmitter()->emitterType)
        {
        case DAVA::ParticleEmitter::EMITTER_ONCIRCLE_VOLUME:
        case DAVA::ParticleEmitter::EMITTER_ONCIRCLE_EDGES:
        {
            DrawSizeCircle(owner, emitter);
            break;
        }
        case DAVA::ParticleEmitter::EMITTER_SHOCKWAVE:
        {
            DrawSizeCircleShockWave(owner, emitter);
            break;
        }

        case DAVA::ParticleEmitter::EMITTER_RECT:
        {
            DrawSizeBox(owner, emitter);
            break;
        }

        default:
            break;
        }
    }
}

void EditorParticlesSystem::Draw()
{
    const SelectableGroup& selection = Selection::GetSelection();
    DAVA::Set<DAVA::ParticleEmitterInstance*> selectedEmitterInstances;
    for (auto instance : selection.ObjectsOfType<DAVA::ParticleEmitterInstance>())
    {
        selectedEmitterInstances.insert(instance);
    }
    DAVA::Set<DAVA::ParticleDragForce*> selectedDragForces;
    for (DAVA::ParticleDragForce* force : selection.ObjectsOfType<DAVA::ParticleDragForce>())
    {
        DrawDragForces(nullptr, force);
    }

    for (auto entity : entities)
    {
        auto effect = static_cast<DAVA::ParticleEffectComponent*>(entity->GetComponent(DAVA::Component::PARTICLE_EFFECT_COMPONENT));
        if (effect != nullptr)
        {
            for (DAVA::uint32 i = 0, e = effect->GetEmittersCount(); i < e; ++i)
            {
                auto instance = effect->GetEmitterInstance(i);
                DrawEmitter(instance, entity, selectedEmitterInstances.count(instance) > 0);
            }
            DrawDebugInfoForEffect(entity);
        }
    }
}

void EditorParticlesSystem::DrawSizeCircleShockWave(DAVA::Entity* effectEntity, DAVA::ParticleEmitterInstance* emitter)
{
    DAVA::float32 time = GetEffectComponent(effectEntity)->GetCurrTime();
    DAVA::float32 emitterRadius = (emitter->GetEmitter()->radius) ? emitter->GetEmitter()->radius->GetValue(time) : 0.0f;
    DAVA::Vector3 emissionVector(0.0f, 0.0f, 1.0f);

    if (emitter->GetEmitter()->emissionVector)
    {
        DAVA::Matrix4 wMat = effectEntity->GetWorldTransform();
        wMat.SetTranslationVector(DAVA::Vector3(0.0f, 0.0f, 0.0f));
        emissionVector = emitter->GetEmitter()->emissionVector->GetValue(time) * wMat;
    }

    auto center = Selectable(emitter).GetWorldTransform().GetTranslationVector();

    auto drawer = GetScene()->GetRenderSystem()->GetDebugDrawer();
    drawer->DrawCircle(center, emissionVector, emitterRadius, 12, DAVA::Color(0.7f, 0.0f, 0.0f, 0.25f), DAVA::RenderHelper::DRAW_SOLID_DEPTH);
}

void EditorParticlesSystem::DrawSizeCircle(DAVA::Entity* effectEntity, DAVA::ParticleEmitterInstance* emitter)
{
    DAVA::float32 emitterRadius = 0.0f;
    DAVA::Vector3 emitterVector;
    DAVA::float32 time = GetEffectComponent(effectEntity)->GetCurrTime();

    if (emitter->GetEmitter()->radius)
    {
        emitterRadius = emitter->GetEmitter()->radius->GetValue(time);
    }

    if (emitter->GetEmitter()->emissionVector)
    {
        DAVA::Matrix4 wMat = effectEntity->GetWorldTransform();
        wMat.SetTranslationVector(DAVA::Vector3(0.0f, 0.0f, 0.0f));
        emitterVector = emitter->GetEmitter()->emissionVector->GetValue(time) * wMat;
    }

    auto center = Selectable(emitter).GetWorldTransform().GetTranslationVector();

    auto drawer = GetScene()->GetRenderSystem()->GetDebugDrawer();
    drawer->DrawCircle(center, emitterVector, emitterRadius, 12,
                       DAVA::Color(0.7f, 0.0f, 0.0f, 0.25f), DAVA::RenderHelper::DRAW_SOLID_DEPTH);
}

void EditorParticlesSystem::DrawSizeBox(DAVA::Entity* effectEntity, DAVA::ParticleEmitterInstance* emitter)
{
    // Default value of emitter size
    DAVA::Vector3 emitterSize;

    DAVA::float32 time = GetEffectComponent(effectEntity)->GetCurrTime();

    if (emitter->GetEmitter()->size)
    {
        emitterSize = emitter->GetEmitter()->size->GetValue(time);
    }

    DAVA::Matrix4 wMat = effectEntity->GetWorldTransform();
    // static DAVA::float32 angle = 0.0f;
    // angle += DAVA::SystemTimer::GetFrameDelta();
    // DAVA::Matrix4 rot;
    // rot.BuildRotation(DAVA::Vector3(0.0f, 1.0f, 0.0f), DAVA::DegToRad(angle));
    wMat.SetTranslationVector(Selectable(emitter).GetWorldTransform().GetTranslationVector());
    // wMat *= rot;

    DAVA::RenderHelper* drawer = GetScene()->GetRenderSystem()->GetDebugDrawer();
    drawer->DrawAABoxCornersTransformed(DAVA::AABBox3(-0.5f * emitterSize, 0.5f * emitterSize), wMat,
                                        DAVA::Color(0.7f, 0.0f, 0.0f, 0.25f), DAVA::RenderHelper::DRAW_SOLID_DEPTH);
}

void EditorParticlesSystem::DrawVectorArrow(DAVA::ParticleEmitterInstance* emitter, DAVA::Vector3 center)
{
    auto effect = emitter->GetOwner();
    if (effect == nullptr)
        return;

    DAVA::Vector3 emitterVector(0.0f, 0.0f, 1.0f);
    if (emitter->GetEmitter()->emissionVector)
    {
        emitterVector = emitter->GetEmitter()->emissionVector->GetValue(effect->GetCurrTime());
        emitterVector.Normalize();
    }

    DAVA::float32 scale = 1.0f;
    HoodSystem* hoodSystem = ((SceneEditor2*)GetScene())->hoodSystem;
    if (hoodSystem != nullptr)
    {
        scale = hoodSystem->GetScale();
    }

    DAVA::float32 arrowSize = scale;
    DAVA::float32 arrowBaseSize = 5.0f;
    emitterVector = (emitterVector * arrowBaseSize * scale);

    DAVA::Matrix4 wMat = effect->GetEntity()->GetWorldTransform();
    wMat.SetTranslationVector(DAVA::Vector3(0, 0, 0));
    TransformPerserveLength(emitterVector, wMat);

    GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawArrow(center, center + emitterVector, arrowSize,
                                                               DAVA::Color(0.7f, 0.0f, 0.0f, 0.25f), DAVA::RenderHelper::DRAW_SOLID_DEPTH);
}

void EditorParticlesSystem::DrawDragForces(DAVA::Entity* effectEntity, DAVA::ParticleDragForce* force)
{
    using namespace DAVA;
    using ForceType = ParticleDragForce::eType;

    if (force->type == ForceType::GRAVITY)
        return;

    if (force->type == ForceType::LORENTZ_FORCE || force->type == ForceType::WIND)
    {
        float32 scale = 1.0f;
        HoodSystem* hoodSystem = ((SceneEditor2*)GetScene())->hoodSystem;
        if (hoodSystem != nullptr)
        {
            scale = hoodSystem->GetScale();
        }
        auto layer = GetDragForceOwner(force);
        auto ent = GetLayerOwner(layer);

        float32 arrowSize = scale;
        float32 arrowBaseSize = 5.0f;
        Vector3 emitterVector = force->direction;

        Matrix4 wMat = ent->GetOwner()->GetEntity()->GetWorldTransform();
        emitterVector = emitterVector * Matrix3(wMat);
        emitterVector.Normalize();
        emitterVector *= arrowBaseSize * scale;
        Vector3 center = force->position * wMat;

        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawArrow(center, center + emitterVector, arrowSize,
            Color(0.7f, 0.7f, 0.0f, 0.35f), RenderHelper::DRAW_SOLID_DEPTH);
    }
    
    RenderHelper* drawer = GetScene()->GetRenderSystem()->GetDebugDrawer();
    if (force->type == ForceType::POINT_GRAVITY)
    {
        Matrix4 wMat = Selectable(force).GetWorldTransform();
        float32 radius = force->pointGravityRadius;
        drawer->DrawIcosahedron(wMat.GetTranslationVector(), radius, Color(0.0f, 0.3f, 0.7f, 0.25f), RenderHelper::DRAW_SOLID_DEPTH);
        drawer->DrawIcosahedron(wMat.GetTranslationVector(), radius, Color(0.0f, 0.15f, 0.35f, 0.35f), RenderHelper::DRAW_WIRE_DEPTH);
    }

    if (force->isInfinityRange)
        return;
    if (force->shape == ParticleDragForce::eShape::BOX)
    {
        auto layer = GetDragForceOwner(force);
        auto ent = GetLayerOwner(layer);

        Matrix4 wMat = ent->GetOwner()->GetEntity()->GetWorldTransform();
        wMat.SetTranslationVector(Selectable(force).GetWorldTransform().GetTranslationVector());

        drawer->DrawAABoxTransformed(AABBox3(-0.5f * force->boxSize, 0.5f * force->boxSize), wMat,
                                     Color(0.0f, 0.7f, 0.3f, 0.25f), RenderHelper::DRAW_SOLID_DEPTH);

        drawer->DrawAABoxTransformed(AABBox3(-0.5f * force->boxSize, 0.5f * force->boxSize), wMat,
                                     Color(0.0f, 0.35f, 0.15f, 0.35f), RenderHelper::DRAW_WIRE_DEPTH);
    }
    else if (force->shape == ParticleDragForce::eShape::SPHERE)
    {
        Matrix4 wMat = Selectable(force).GetWorldTransform();
        float32 radius = force->radius;
        drawer->DrawIcosahedron(wMat.GetTranslationVector(), radius, Color(0.0f, 0.7f, 0.3f, 0.25f), RenderHelper::DRAW_SOLID_DEPTH);
        drawer->DrawIcosahedron(wMat.GetTranslationVector(), radius, Color(0.0f, 0.35f, 0.15f, 0.35f), RenderHelper::DRAW_WIRE_DEPTH);
    }
}

void EditorParticlesSystem::AddEntity(DAVA::Entity* entity)
{
    entities.push_back(entity);
}

void EditorParticlesSystem::RemoveEntity(DAVA::Entity* entity)
{
    DAVA::FindAndRemoveExchangingWithLast(entities, entity);
}

void EditorParticlesSystem::RestartParticleEffects()
{
    for (DAVA::Entity* entity : entities)
    {
        DAVA::ParticleEffectComponent* effectComponent = DAVA::GetEffectComponent(entity);
        DVASSERT(effectComponent);
        if (!effectComponent->IsStopped())
        {
            effectComponent->Restart();
        }
    }
}

DAVA::ParticleLayer* EditorParticlesSystem::GetForceOwner(DAVA::ParticleForce* force) const
{
    DAVA::Function<DAVA::ParticleLayer*(DAVA::ParticleEmitter*, DAVA::ParticleForce*)> getForceOwner = [&getForceOwner](DAVA::ParticleEmitter* emitter, DAVA::ParticleForce* force) -> DAVA::ParticleLayer*
    {
        for (DAVA::ParticleLayer* layer : emitter->layers)
        {
            if (layer->type == DAVA::ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
            {
                DAVA::ParticleLayer* foundLayer = getForceOwner(layer->innerEmitter, force);
                if (foundLayer != nullptr)
                {
                    return foundLayer;
                }
            }

            if (std::find(layer->forces.begin(), layer->forces.end(), force) != layer->forces.end())
            {
                return layer;
            }
        }

        return nullptr;
    };

    for (DAVA::Entity* entity : entities)
    {
        DAVA::ParticleEffectComponent* effectComponent = DAVA::GetEffectComponent(entity);
        DAVA::uint32 emittersCount = effectComponent->GetEmittersCount();
        for (DAVA::uint32 id = 0; id < emittersCount; ++id)
        {
            DAVA::ParticleEmitterInstance* emitterInstance = effectComponent->GetEmitterInstance(id);
            DVASSERT(emitterInstance != nullptr);

            DAVA::ParticleEmitter* emitter = emitterInstance->GetEmitter();
            DVASSERT(emitter != nullptr);

            DAVA::ParticleLayer* owner = getForceOwner(emitter, force);
            if (owner != nullptr)
            {
                return owner;
            }
        }
    }

    return nullptr;
}

DAVA::ParticleLayer* EditorParticlesSystem::GetDragForceOwner(DAVA::ParticleDragForce* force) const
{
    DAVA::Function<DAVA::ParticleLayer*(DAVA::ParticleEmitter*, DAVA::ParticleDragForce*)> getForceOwner = [&getForceOwner](DAVA::ParticleEmitter* emitter, DAVA::ParticleDragForce* force) -> DAVA::ParticleLayer*
    {
        for (DAVA::ParticleLayer* layer : emitter->layers)
        {
            if (layer->type == DAVA::ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
            {
                DAVA::ParticleLayer* foundLayer = getForceOwner(layer->innerEmitter, force);
                if (foundLayer != nullptr)
                {
                    return foundLayer;
                }
            }

            if (std::find(layer->GetDragForces().begin(), layer->GetDragForces().end(), force) != layer->GetDragForces().end())
            {
                return layer;
            }
        }

        return nullptr;
    };

    for (DAVA::Entity* entity : entities)
    {
        DAVA::ParticleEffectComponent* effectComponent = DAVA::GetEffectComponent(entity);
        DAVA::uint32 emittersCount = effectComponent->GetEmittersCount();
        for (DAVA::uint32 id = 0; id < emittersCount; ++id)
        {
            DAVA::ParticleEmitterInstance* emitterInstance = effectComponent->GetEmitterInstance(id);
            DVASSERT(emitterInstance != nullptr);

            DAVA::ParticleEmitter* emitter = emitterInstance->GetEmitter();
            DVASSERT(emitter != nullptr);

            DAVA::ParticleLayer* owner = getForceOwner(emitter, force);
            if (owner != nullptr)
            {
                return owner;
            }
        }
    }

    return nullptr;
}

DAVA::ParticleEmitterInstance* EditorParticlesSystem::GetLayerOwner(DAVA::ParticleLayer* layer) const
{
    DAVA::Function<bool(DAVA::ParticleEmitter*, DAVA::ParticleLayer*)> hasLayerOwner = [&hasLayerOwner](DAVA::ParticleEmitter* emitter, DAVA::ParticleLayer* layer) -> bool
    {
        for (DAVA::ParticleLayer* l : emitter->layers)
        {
            if (l == layer)
            {
                return true;
            }

            if (l->type == DAVA::ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
            {
                bool found = hasLayerOwner(l->innerEmitter, layer);
                if (found)
                {
                    return true;
                }
            }
        }

        return false;
    };

    for (DAVA::Entity* entity : entities)
    {
        DAVA::ParticleEffectComponent* effectComponent = DAVA::GetEffectComponent(entity);
        DAVA::uint32 emittersCount = effectComponent->GetEmittersCount();
        for (DAVA::uint32 id = 0; id < emittersCount; ++id)
        {
            DAVA::ParticleEmitterInstance* emitterInstance = effectComponent->GetEmitterInstance(id);
            DVASSERT(emitterInstance != nullptr);

            DAVA::ParticleEmitter* emitter = emitterInstance->GetEmitter();
            DVASSERT(emitter != nullptr);

            if (hasLayerOwner(emitter, layer))
            {
                return emitterInstance;
            }
        }
    }

    return nullptr;
}

void EditorParticlesSystem::ProcessCommand(const RECommandNotificationObject& commandNotification)
{
    SceneEditor2* activeScene = static_cast<SceneEditor2*>(GetScene());
    auto processSingleCommand = [&activeScene, this](const RECommand* command, bool redo)
    {
        switch (command->GetID())
        {
        case CMDID_PARTICLE_EMITTER_UPDATE:
        {
            const CommandUpdateEmitter* castedCmd = static_cast<const CommandUpdateEmitter*>(command);
            SceneSignals::Instance()->EmitParticleEmitterValueChanged(activeScene, castedCmd->GetEmitterInstance());
            break;
        }

        case CMDID_PARTICLE_LAYER_UPDATE:
        {
            const CommandUpdateParticleLayerBase* castedCmd = static_cast<const CommandUpdateParticleLayerBase*>(command);
            SceneSignals::Instance()->EmitParticleLayerValueChanged(activeScene, castedCmd->GetLayer());
            break;
        }
        case CMDID_PARTICLE_LAYER_CHANGED_MATERIAL_VALUES:
        {
            EmitValueChanged<CommandChangeLayerMaterialProperties>(command, activeScene);
            break;
        }
        case CMDID_PARTICLE_LAYER_CHANGED_FLOW_VALUES:
        {
            EmitValueChanged<CommandChangeFlowProperties>(command, activeScene);
            break;
        }
        case CMDID_PARTICLE_LAYER_CHANGED_NOISE_VALUES:
        {
            EmitValueChanged<CommandChangeNoiseProperties>(command, activeScene);
            break;
        }
        case CMDID_PARTICLE_LAYER_CHANGED_FRES_TO_ALPHA_VALUES:
        {
            EmitValueChanged<CommandChangeFresnelToAlphaProperties>(command, activeScene);
            break;
        }
        case CMDID_PARTICLE_LAYER_CHANGED_STRIPE_VALUES:
        {
            EmitValueChanged<CommandChangeParticlesStripeProperties>(command, activeScene);
            break;
        }
        case CMDID_PARTICLE_LAYER_CHANGED_ALPHA_REMAP:
        {
            EmitValueChanged<CommandChangeAlphaRemapProperties>(command, activeScene);
            break;
        }

        case CMDID_PARTILCE_LAYER_UPDATE_TIME:
        case CMDID_PARTICLE_LAYER_UPDATE_ENABLED:
        {
            const CommandUpdateParticleLayerBase* castedCmd = static_cast<const CommandUpdateParticleLayerBase*>(command);
            SceneSignals::Instance()->EmitParticleLayerValueChanged(activeScene, castedCmd->GetLayer());
            break;
        }

        case CMDID_PARTICLE_FORCE_UPDATE:
        {
            const CommandUpdateParticleForce* castedCmd = static_cast<const CommandUpdateParticleForce*>(command);
            SceneSignals::Instance()->EmitParticleForceValueChanged(activeScene, castedCmd->GetLayer(), castedCmd->GetForceIndex());
            break;
        }
        case CMDID_PARTICLE_DRAG_FORCE_UPDATE:
        {
            const CommandUpdateParticleDragForce* castedCmd = static_cast<const CommandUpdateParticleDragForce*>(command);
            SceneSignals::Instance()->EmitParticleDragForceValueChanged(activeScene, castedCmd->GetLayer(), castedCmd->GetForceIndex());
        }

        case CMDID_PARTICLE_EFFECT_START_STOP:
        {
            const CommandStartStopParticleEffect* castedCmd = static_cast<const CommandStartStopParticleEffect*>(command);
            SceneSignals::Instance()->EmitParticleEffectStateChanged(activeScene, castedCmd->GetEntity(), castedCmd->GetStarted());
            break;
        }

        case CMDID_PARTICLE_EFFECT_RESTART:
        {
            const CommandRestartParticleEffect* castedCmd = static_cast<const CommandRestartParticleEffect*>(command);

            // An effect was stopped and then started.
            SceneSignals::Instance()->EmitParticleEffectStateChanged(activeScene, castedCmd->GetEntity(), false);
            SceneSignals::Instance()->EmitParticleEffectStateChanged(activeScene, castedCmd->GetEntity(), true);
            break;
        }

        case CMDID_PARTICLE_EMITTER_LOAD_FROM_YAML:
        {
            const CommandLoadParticleEmitterFromYaml* castedCmd = static_cast<const CommandLoadParticleEmitterFromYaml*>(command);
            SceneSignals::Instance()->EmitParticleEmitterLoaded(activeScene, castedCmd->GetEmitterInstance());
            break;
        }

        case CMDID_PARTICLE_EMITTER_SAVE_TO_YAML:
        {
            const CommandSaveParticleEmitterToYaml* castedCmd = static_cast<const CommandSaveParticleEmitterToYaml*>(command);
            SceneSignals::Instance()->EmitParticleEmitterSaved(activeScene, castedCmd->GetEmitterInstance());
            break;
        }

        case CMDID_PARTICLE_INNER_EMITTER_LOAD_FROM_YAML:
        {
            const CommandLoadInnerParticleEmitterFromYaml* castedCmd = static_cast<const CommandLoadInnerParticleEmitterFromYaml*>(command);
            SceneSignals::Instance()->EmitParticleEmitterLoaded(activeScene, castedCmd->GetEmitterInstance());
            break;
        }

        case CMDID_PARTICLE_INNER_EMITTER_SAVE_TO_YAML:
        {
            const CommandSaveInnerParticleEmitterToYaml* castedCmd = static_cast<const CommandSaveInnerParticleEmitterToYaml*>(command);
            SceneSignals::Instance()->EmitParticleEmitterSaved(activeScene, castedCmd->GetEmitterInstance());
            break;
        }

        case CMDID_PARTICLE_EMITTER_LAYER_ADD:
        {
            const CommandAddParticleEmitterLayer* castedCmd = static_cast<const CommandAddParticleEmitterLayer*>(command);
            SceneSignals::Instance()->EmitParticleLayerAdded(activeScene, castedCmd->GetParentEmitter(), castedCmd->GetCreatedLayer());
            break;
        }
        // Return to this code when implementing Layer popup menus.
        /*
        case CMDID_REMOVE_PARTICLE_EMITTER_LAYER:
        {
        const CommandRemoveParticleEmitterLayer* castedCmd = static_cast<const CommandRemoveParticleEmitterLayer*>(command);
        SceneSignals::Instance()->EmitParticleLayerRemoved(activeScene, castedCmd->GetEmitter());
        break;
        }
        */
        default:
            break;
        }
    };

    static const DAVA::Vector<DAVA::uint32> commandIDs =
    {
      CMDID_PARTICLE_EMITTER_UPDATE, CMDID_PARTICLE_LAYER_UPDATE, CMDID_PARTICLE_LAYER_CHANGED_MATERIAL_VALUES, CMDID_PARTICLE_LAYER_CHANGED_FLOW_VALUES, CMDID_PARTICLE_LAYER_CHANGED_NOISE_VALUES, CMDID_PARTICLE_LAYER_CHANGED_FRES_TO_ALPHA_VALUES, CMDID_PARTICLE_LAYER_CHANGED_STRIPE_VALUES, CMDID_PARTICLE_LAYER_CHANGED_ALPHA_REMAP,
      CMDID_PARTILCE_LAYER_UPDATE_TIME, CMDID_PARTICLE_LAYER_UPDATE_ENABLED, CMDID_PARTICLE_FORCE_UPDATE, CMDID_PARTICLE_DRAG_FORCE_UPDATE,
      CMDID_PARTICLE_EFFECT_START_STOP, CMDID_PARTICLE_EFFECT_RESTART, CMDID_PARTICLE_EMITTER_LOAD_FROM_YAML,
      CMDID_PARTICLE_EMITTER_SAVE_TO_YAML,
      CMDID_PARTICLE_INNER_EMITTER_LOAD_FROM_YAML, CMDID_PARTICLE_INNER_EMITTER_SAVE_TO_YAML,
      //            CMDID_REMOVE_PARTICLE_EMITTER_LAYER,
      CMDID_PARTICLE_EMITTER_LAYER_ADD
    };

    if (commandNotification.MatchCommandIDs(commandIDs))
    {
        commandNotification.ExecuteForAllCommands(processSingleCommand);
    }
}
