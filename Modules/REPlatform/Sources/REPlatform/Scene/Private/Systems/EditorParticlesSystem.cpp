#include "REPlatform/Scene/Systems/EditorParticlesSystem.h"
#include "REPlatform/Scene/Systems/CollisionSystem.h"
#include "REPlatform/Scene/Systems/HoodSystem.h"
#include "REPlatform/Scene/Systems/SelectionSystem.h"
#include "REPlatform/Scene/SceneEditor2.h"

#include "REPlatform/Commands/ParticleEditorCommands.h"
#include "REPlatform/Commands/ParticleLayerCommands.h"
#include "REPlatform/Commands/RECommandNotificationObject.h"

#include <Base/BaseTypes.h>
#include <Entity/Component.h>
#include <Particles/ParticleEmitter.h>
#include <Particles/ParticleEmitterInstance.h>
#include <Scene3D/Components/ComponentHelpers.h>
#include <Scene3D/Components/ParticleEffectComponent.h>
#include <Scene3D/Components/RenderComponent.h>

namespace DAVA
{
EditorParticlesSystem::EditorParticlesSystem(Scene* scene)
    : SceneSystem(scene)
{
}

void EditorParticlesSystem::DrawDebugInfoForEffect(Entity* effectEntity)
{
    DVASSERT(effectEntity != nullptr);

    Scene* scene = GetScene();
    SceneCollisionSystem* collisionSystem = scene->GetSystem<SceneCollisionSystem>();

    AABBox3 worldBox;
    AABBox3 collBox = collisionSystem->GetBoundingBox(effectEntity);
    DVASSERT(!collBox.IsEmpty());
    collBox.GetTransformedBox(effectEntity->GetWorldTransform(), worldBox);
    float32 radius = (collBox.max - collBox.min).Length() / 3;
    scene->GetRenderSystem()->GetDebugDrawer()->DrawIcosahedron(worldBox.GetCenter(), radius, Color(0.9f, 0.9f, 0.9f, 0.35f), RenderHelper::DRAW_SOLID_DEPTH);
}

void EditorParticlesSystem::DrawEmitter(ParticleEmitterInstance* emitter, Entity* owner, bool selected)
{
    DVASSERT((emitter != nullptr) && (owner != nullptr));

    Scene* scene = GetScene();
    SceneCollisionSystem* collisionSystem = scene->GetSystem<SceneCollisionSystem>();

    Vector3 center = emitter->GetSpawnPosition();
    TransformPerserveLength(center, Matrix3(owner->GetWorldTransform()));
    center += owner->GetWorldTransform().GetTranslationVector();

    AABBox3 boundingBox = collisionSystem->GetBoundingBox(owner);
    DVASSERT(!boundingBox.IsEmpty());
    float32 radius = (boundingBox.max - boundingBox.min).Length() / 3;
    scene->GetRenderSystem()->GetDebugDrawer()->DrawIcosahedron(center, radius, Color(1.0f, 1.0f, 1.0f, 0.5f), RenderHelper::DRAW_SOLID_DEPTH);

    if (selected)
    {
        DrawVectorArrow(emitter, center);

        switch (emitter->GetEmitter()->emitterType)
        {
        case ParticleEmitter::EMITTER_ONCIRCLE_VOLUME:
        case ParticleEmitter::EMITTER_ONCIRCLE_EDGES:
        {
            DrawSizeCircle(owner, emitter);
            break;
        }
        case ParticleEmitter::EMITTER_SHOCKWAVE:
        {
            DrawSizeCircleShockWave(owner, emitter);
            break;
        }

        case ParticleEmitter::EMITTER_RECT:
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
    const SelectableGroup& selection = GetScene()->GetSystem<SelectionSystem>()->GetSelection();
    Set<ParticleEmitterInstance*> selectedEmitterInstances;
    for (auto instance : selection.ObjectsOfType<ParticleEmitterInstance>())
    {
        selectedEmitterInstances.insert(instance);
    }

    for (auto entity : entities)
    {
        auto effect = static_cast<ParticleEffectComponent*>(entity->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
        if (effect != nullptr)
        {
            for (uint32 i = 0, e = effect->GetEmittersCount(); i < e; ++i)
            {
                auto instance = effect->GetEmitterInstance(i);
                DrawEmitter(instance, entity, selectedEmitterInstances.count(instance) > 0);
            }
            DrawDebugInfoForEffect(entity);
        }
    }
}

void EditorParticlesSystem::ProcessCommand(const RECommandNotificationObject& commandNotification)
{
    if (commandNotification.MatchCommandTypes<CommandChangeLayerMaterialProperties,
                                              CommandChangeFlowProperties,
                                              CommandChangeNoiseProperties,
                                              CommandChangeFresnelToAlphaProperties,
                                              CommandChangeParticlesStripeProperties,
                                              CommandChangeAlphaRemapProperties,
                                              CommandUpdateParticleLayerBase>())
    {
        RestartParticleEffects();
    }
}

void EditorParticlesSystem::DrawSizeCircleShockWave(Entity* effectEntity, ParticleEmitterInstance* emitter)
{
    float32 time = GetEffectComponent(effectEntity)->GetCurrTime();
    float32 emitterRadius = (emitter->GetEmitter()->radius) ? emitter->GetEmitter()->radius->GetValue(time) : 0.0f;
    Vector3 emissionVector(0.0f, 0.0f, 1.0f);

    if (emitter->GetEmitter()->emissionVector)
    {
        Matrix4 wMat = effectEntity->GetWorldTransform();
        wMat.SetTranslationVector(Vector3(0.0f, 0.0f, 0.0f));
        emissionVector = emitter->GetEmitter()->emissionVector->GetValue(time) * wMat;
    }

    auto center = Selectable(emitter).GetWorldTransform().GetTranslationVector();

    auto drawer = GetScene()->GetRenderSystem()->GetDebugDrawer();
    drawer->DrawCircle(center, emissionVector, emitterRadius, 12, Color(0.7f, 0.0f, 0.0f, 0.25f), RenderHelper::DRAW_SOLID_DEPTH);
}

void EditorParticlesSystem::DrawSizeCircle(Entity* effectEntity, ParticleEmitterInstance* emitter)
{
    float32 emitterRadius = 0.0f;
    Vector3 emitterVector;
    float32 time = GetEffectComponent(effectEntity)->GetCurrTime();

    if (emitter->GetEmitter()->radius)
    {
        emitterRadius = emitter->GetEmitter()->radius->GetValue(time);
    }

    if (emitter->GetEmitter()->emissionVector)
    {
        Matrix4 wMat = effectEntity->GetWorldTransform();
        wMat.SetTranslationVector(Vector3(0.0f, 0.0f, 0.0f));
        emitterVector = emitter->GetEmitter()->emissionVector->GetValue(time) * wMat;
    }

    auto center = Selectable(emitter).GetWorldTransform().GetTranslationVector();

    auto drawer = GetScene()->GetRenderSystem()->GetDebugDrawer();
    drawer->DrawCircle(center, emitterVector, emitterRadius, 12,
                       Color(0.7f, 0.0f, 0.0f, 0.25f), RenderHelper::DRAW_SOLID_DEPTH);
}

void EditorParticlesSystem::DrawSizeBox(Entity* effectEntity, ParticleEmitterInstance* emitter)
{
    // Default value of emitter size
    Vector3 emitterSize;

    float32 time = GetEffectComponent(effectEntity)->GetCurrTime();

    if (emitter->GetEmitter()->size)
    {
        emitterSize = emitter->GetEmitter()->size->GetValue(time);
    }

    Matrix4 wMat = effectEntity->GetWorldTransform();
    wMat.SetTranslationVector(Selectable(emitter).GetWorldTransform().GetTranslationVector());

    RenderHelper* drawer = GetScene()->GetRenderSystem()->GetDebugDrawer();
    drawer->DrawAABoxTransformed(AABBox3(-0.5f * emitterSize, 0.5f * emitterSize), wMat,
                                 Color(0.7f, 0.0f, 0.0f, 0.25f), RenderHelper::DRAW_SOLID_DEPTH);
}

void EditorParticlesSystem::DrawVectorArrow(ParticleEmitterInstance* emitter, Vector3 center)
{
    auto effect = emitter->GetOwner();
    if (effect == nullptr)
        return;

    Vector3 emitterVector(0.0f, 0.0f, 1.0f);
    if (emitter->GetEmitter()->emissionVector)
    {
        emitterVector = emitter->GetEmitter()->emissionVector->GetValue(effect->GetCurrTime());
        emitterVector.Normalize();
    }

    float32 scale = 1.0f;
    HoodSystem* hoodSystem = GetScene()->GetSystem<HoodSystem>();
    if (hoodSystem != nullptr)
    {
        scale = hoodSystem->GetScale();
    }

    float32 arrowSize = scale;
    float32 arrowBaseSize = 5.0f;
    emitterVector = (emitterVector * arrowBaseSize * scale);

    Matrix4 wMat = effect->GetEntity()->GetWorldTransform();
    wMat.SetTranslationVector(Vector3(0, 0, 0));
    TransformPerserveLength(emitterVector, wMat);

    GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawArrow(center, center + emitterVector, arrowSize,
                                                               Color(0.7f, 0.0f, 0.0f, 0.25f), RenderHelper::DRAW_SOLID_DEPTH);
}

void EditorParticlesSystem::AddEntity(Entity* entity)
{
    entities.push_back(entity);
}

void EditorParticlesSystem::RemoveEntity(Entity* entity)
{
    FindAndRemoveExchangingWithLast(entities, entity);
}

void EditorParticlesSystem::PrepareForRemove()
{
    entities.clear();
}

void EditorParticlesSystem::RestartParticleEffects()
{
    for (Entity* entity : entities)
    {
        ParticleEffectComponent* effectComponent = GetEffectComponent(entity);
        DVASSERT(effectComponent);
        if (!effectComponent->IsStopped())
        {
            effectComponent->Restart();
        }
    }
}

ParticleLayer* EditorParticlesSystem::GetForceOwner(ParticleForce* force) const
{
    Function<ParticleLayer*(ParticleEmitter*, ParticleForce*)> getForceOwner = [&getForceOwner](ParticleEmitter* emitter, ParticleForce* force) -> ParticleLayer*
    {
        for (ParticleLayer* layer : emitter->layers)
        {
            if (layer->type == ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
            {
                ParticleLayer* foundLayer = getForceOwner(layer->innerEmitter, force);
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

    for (Entity* entity : entities)
    {
        ParticleEffectComponent* effectComponent = GetEffectComponent(entity);
        uint32 emittersCount = effectComponent->GetEmittersCount();
        for (uint32 id = 0; id < emittersCount; ++id)
        {
            ParticleEmitterInstance* emitterInstance = effectComponent->GetEmitterInstance(id);
            DVASSERT(emitterInstance != nullptr);

            ParticleEmitter* emitter = emitterInstance->GetEmitter();
            DVASSERT(emitter != nullptr);

            ParticleLayer* owner = getForceOwner(emitter, force);
            if (owner != nullptr)
            {
                return owner;
            }
        }
    }

    return nullptr;
}

ParticleEmitterInstance* EditorParticlesSystem::GetLayerOwner(ParticleLayer* layer) const
{
    Function<bool(ParticleEmitter*, ParticleLayer*)> hasLayerOwner = [&hasLayerOwner](ParticleEmitter* emitter, ParticleLayer* layer) -> bool
    {
        for (ParticleLayer* l : emitter->layers)
        {
            if (l == layer)
            {
                return true;
            }

            if (l->type == ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
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

    for (Entity* entity : entities)
    {
        ParticleEffectComponent* effectComponent = GetEffectComponent(entity);
        uint32 emittersCount = effectComponent->GetEmittersCount();
        for (uint32 id = 0; id < emittersCount; ++id)
        {
            ParticleEmitterInstance* emitterInstance = effectComponent->GetEmitterInstance(id);
            DVASSERT(emitterInstance != nullptr);

            ParticleEmitter* emitter = emitterInstance->GetEmitter();
            DVASSERT(emitter != nullptr);

            if (hasLayerOwner(emitter, layer))
            {
                return emitterInstance;
            }
        }
    }

    return nullptr;
}
} // namespace DAVA
