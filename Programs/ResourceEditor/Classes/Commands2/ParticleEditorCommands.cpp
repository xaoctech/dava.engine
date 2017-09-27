#include "ParticleEditorCommands.h"
#include "Commands2/RECommandIDs.h"

#include "DAVAEngine.h"
#include "Deprecated/ParticlesEditorNodeNameHelper.h"

#include "Main/QtUtils.h"
#include "StringConstants.h"

#include <QString>

#include "Scene3D/Components/ParticleEffectComponent.h"

using namespace DAVA;

void AddNewForceToLayer(ParticleLayer* layer, ParticleDragForce::eType forceType);

CommandUpdateEffect::CommandUpdateEffect(ParticleEffectComponent* effect)
    : CommandAction(CMDID_PARTICLE_EFFECT_UPDATE)
    , particleEffect(effect)
{
}

void CommandUpdateEffect::Init(float32 speed)
{
    playbackSpeed = speed;
}

void CommandUpdateEffect::Redo()
{
    DVASSERT(particleEffect);
    particleEffect->SetPlaybackSpeed(playbackSpeed);
}

CommandUpdateEmitter::CommandUpdateEmitter(ParticleEmitterInstance* emitter_)
    : CommandAction(CMDID_PARTICLE_EMITTER_UPDATE)
    , instance(emitter_)
{
}

void CommandUpdateEmitter::Init(const FastName& name,
                                ParticleEmitter::eType emitterType,
                                RefPtr<PropertyLine<float32>> emissionRange,
                                RefPtr<PropertyLine<Vector3>> emissionVector,
                                RefPtr<PropertyLine<float32>> radius,
                                RefPtr<PropertyLine<float32>> emissionAngle,
                                RefPtr<PropertyLine<float32>> emissionAngleVariation,
                                RefPtr<PropertyLine<Color>> colorOverLife,
                                RefPtr<PropertyLine<Vector3>> size,
                                float32 life,
                                bool isShortEffect)
{
    this->name = name;
    this->emitterType = emitterType;
    this->emissionRange = emissionRange;
    this->emissionVector = emissionVector;
    this->radius = radius;
    this->emissionAngle = emissionAngle;
    this->emissionAngleVariation = emissionAngleVariation;
    this->colorOverLife = colorOverLife;
    this->size = size;
    this->life = life;
    this->isShortEffect = isShortEffect;
}

void CommandUpdateEmitter::Redo()
{
    DVASSERT(instance);
    auto emitter = instance->GetEmitter();
    emitter->name = name;
    emitter->emitterType = emitterType;
    PropertyLineHelper::SetValueLine(emitter->emissionRange, emissionRange);
    PropertyLineHelper::SetValueLine(emitter->emissionVector, emissionVector);
    PropertyLineHelper::SetValueLine(emitter->radius, radius);
    PropertyLineHelper::SetValueLine(emitter->colorOverLife, colorOverLife);
    PropertyLineHelper::SetValueLine(emitter->size, size);
    PropertyLineHelper::SetValueLine(emitter->emissionAngle, emissionAngle);
    PropertyLineHelper::SetValueLine(emitter->emissionAngleVariation, emissionAngleVariation);
    emitter->shortEffect = isShortEffect;
}

CommandUpdateParticleLayer::CommandUpdateParticleLayer(ParticleEmitterInstance* emitter, ParticleLayer* layer)
    : CommandUpdateParticleLayerBase(CMDID_PARTICLE_LAYER_UPDATE)
{
    this->emitter = emitter;
    this->layer = layer;
}

void CommandUpdateParticleLayer::Init(const String& layerName,
                                      ParticleLayer::eType layerType,
                                      ParticleLayer::eDegradeStrategy degradeStrategy,
                                      bool isDisabled,
                                      bool inheritPosition,
                                      bool isLong,
                                      float32 scaleVelocityBase,
                                      float32 scaleVelocityFactor,
                                      bool isLooped,
                                      int32 particleOrientation,
                                      RefPtr<PropertyLine<float32>> life,
                                      RefPtr<PropertyLine<float32>> lifeVariation,
                                      RefPtr<PropertyLine<float32>> number,
                                      RefPtr<PropertyLine<float32>> numberVariation,
                                      RefPtr<PropertyLine<Vector2>> size,
                                      RefPtr<PropertyLine<Vector2>> sizeVariation,
                                      RefPtr<PropertyLine<Vector2>> sizeOverLife,
                                      RefPtr<PropertyLine<float32>> velocity,
                                      RefPtr<PropertyLine<float32>> velocityVariation,
                                      RefPtr<PropertyLine<float32>> velocityOverLife,
                                      RefPtr<PropertyLine<float32>> spin,
                                      RefPtr<PropertyLine<float32>> spinVariation,
                                      RefPtr<PropertyLine<float32>> spinOverLife,
                                      bool randomSpinDirection,

                                      RefPtr<PropertyLine<Color>> colorRandom,
                                      RefPtr<PropertyLine<float32>> alphaOverLife,
                                      RefPtr<PropertyLine<Color>> colorOverLife,
                                      RefPtr<PropertyLine<float32>> angle,
                                      RefPtr<PropertyLine<float32>> angleVariation,

                                      float32 startTime,
                                      float32 endTime,
                                      float32 deltaTime,
                                      float32 deltaVariation,
                                      float32 loopEndTime,
                                      float32 loopVariation,
                                      bool frameOverLifeEnabled,
                                      float32 frameOverLifeFPS,
                                      bool randomFrameOnStart,
                                      bool loopSpriteAnimation,
                                      RefPtr<PropertyLine<float32>> animSpeedOverLife,

                                      float32 pivotPointX,
                                      float32 pivotPointY)
{
    this->layerName = layerName;
    this->layerType = layerType;
    this->degradeStrategy = degradeStrategy;
    this->isDisabled = isDisabled;
    this->inheritPosition = inheritPosition;
    this->isLooped = isLooped;
    this->isLong = isLong;
    this->scaleVelocityBase = scaleVelocityBase;
    this->scaleVelocityFactor = scaleVelocityFactor;
    this->life = life;
    this->lifeVariation = lifeVariation;
    this->number = number;
    this->numberVariation = numberVariation;
    this->size = size;
    this->sizeVariation = sizeVariation;
    this->sizeOverLife = sizeOverLife;
    this->velocity = velocity;
    this->velocityVariation = velocityVariation;
    this->velocityOverLife = velocityOverLife;
    this->spin = spin;
    this->spinVariation = spinVariation;
    this->spinOverLife = spinOverLife;
    this->randomSpinDirection = randomSpinDirection;
    this->particleOrientation = particleOrientation;

    this->colorRandom = colorRandom;
    this->alphaOverLife = alphaOverLife;
    this->colorOverLife = colorOverLife;
    this->angle = angle;
    this->angleVariation = angleVariation;

    this->startTime = startTime;
    this->endTime = endTime;
    this->deltaTime = deltaTime;
    this->deltaVariation = deltaVariation;
    this->loopEndTime = loopEndTime;
    this->loopVariation = loopVariation;
    this->frameOverLifeEnabled = frameOverLifeEnabled;
    this->frameOverLifeFPS = frameOverLifeFPS;
    this->randomFrameOnStart = randomFrameOnStart;
    this->loopSpriteAnimation = loopSpriteAnimation;
    this->animSpeedOverLife = animSpeedOverLife;

    this->pivotPointX = pivotPointX;
    this->pivotPointY = pivotPointY;
}

void CommandUpdateParticleLayer::Redo()
{
    layer->layerName = layerName;
    layer->degradeStrategy = degradeStrategy;
    layer->isDisabled = isDisabled;
    layer->SetInheritPosition(inheritPosition);
    layer->isLong = isLong;
    layer->scaleVelocityBase = scaleVelocityBase;
    layer->scaleVelocityFactor = scaleVelocityFactor;
    layer->isLooped = isLooped;
    PropertyLineHelper::SetValueLine(layer->life, life);
    PropertyLineHelper::SetValueLine(layer->lifeVariation, lifeVariation);
    PropertyLineHelper::SetValueLine(layer->number, number);
    PropertyLineHelper::SetValueLine(layer->numberVariation, numberVariation);
    PropertyLineHelper::SetValueLine(layer->size, size);
    PropertyLineHelper::SetValueLine(layer->sizeVariation, sizeVariation);
    PropertyLineHelper::SetValueLine(layer->sizeOverLifeXY, sizeOverLife);
    PropertyLineHelper::SetValueLine(layer->velocity, velocity);
    PropertyLineHelper::SetValueLine(layer->velocityVariation, velocityVariation);
    PropertyLineHelper::SetValueLine(layer->velocityOverLife, velocityOverLife);
    PropertyLineHelper::SetValueLine(layer->spin, spin);
    PropertyLineHelper::SetValueLine(layer->spinVariation, spinVariation);
    PropertyLineHelper::SetValueLine(layer->spinOverLife, spinOverLife);
    layer->randomSpinDirection = randomSpinDirection;
    layer->particleOrientation = particleOrientation;

    PropertyLineHelper::SetValueLine(layer->colorRandom, colorRandom);
    PropertyLineHelper::SetValueLine(layer->alphaOverLife, alphaOverLife);
    PropertyLineHelper::SetValueLine(layer->colorOverLife, colorOverLife);

    layer->frameOverLifeEnabled = frameOverLifeEnabled;
    layer->frameOverLifeFPS = frameOverLifeFPS;
    layer->randomFrameOnStart = randomFrameOnStart;
    layer->loopSpriteAnimation = loopSpriteAnimation;
    PropertyLineHelper::SetValueLine(layer->animSpeedOverLife, animSpeedOverLife);

    PropertyLineHelper::SetValueLine(layer->angle, angle);
    PropertyLineHelper::SetValueLine(layer->angleVariation, angleVariation);

    layer->UpdateLayerTime(startTime, endTime);
    layer->deltaTime = deltaTime;
    layer->deltaVariation = deltaVariation;
    layer->loopEndTime = loopEndTime;
    layer->loopVariation = loopVariation;

    layer->SetPivotPoint(Vector2(pivotPointX, pivotPointY));

    // The same is for emitter type.
    if (layer->type != layerType)
    {
        if (layerType == ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
        {
            SafeRelease(layer->innerEmitter);
        }
        layer->type = layerType;
        if (layerType == ParticleLayer::TYPE_SUPEREMITTER_PARTICLES)
        {
            SafeRelease(layer->innerEmitter);
            layer->innerEmitter = new ParticleEmitter();
            if (!layer->innerEmitterPath.IsEmpty())
            {
                layer->innerEmitter->LoadFromYaml(layer->innerEmitterPath);
            }
        }
        //TODO: restart in effect
    }
    layer->isLong = isLong;
}

CommandUpdateParticleLayerTime::CommandUpdateParticleLayerTime(ParticleLayer* layer)
    : CommandUpdateParticleLayerBase(CMDID_PARTILCE_LAYER_UPDATE_TIME)
{
    this->layer = layer;
}

void CommandUpdateParticleLayerTime::Init(float32 startTime, float32 endTime)
{
    this->startTime = startTime;
    this->endTime = endTime;
}

void CommandUpdateParticleLayerTime::Redo()
{
    layer->UpdateLayerTime(startTime, endTime);
}

CommandUpdateParticleLayerEnabled::CommandUpdateParticleLayerEnabled(ParticleLayer* layer, bool isEnabled)
    : CommandUpdateParticleLayerBase(CMDID_PARTICLE_LAYER_UPDATE_ENABLED)
{
    this->layer = layer;
    this->isEnabled = isEnabled;
}

void CommandUpdateParticleLayerEnabled::Redo()
{
    if (layer)
    {
        layer->isDisabled = !isEnabled;
    }
}

CommandUpdateParticleLayerLods::CommandUpdateParticleLayerLods(ParticleLayer* layer, const Vector<bool>& lods)
    : CommandUpdateParticleLayerBase(CMDID_PARTICLE_LAYER_UPDATE_LODS)
{
    this->layer = layer;
    this->lods = lods;
}

void CommandUpdateParticleLayerLods::Redo()
{
    if (layer == nullptr)
        return;

    for (DAVA::size_type i = 0; i < lods.size(); i++)
    {
        layer->SetLodActive(static_cast<DAVA::int32>(i), lods[i]);
    }
}

CommandUpdateParticleForce::CommandUpdateParticleForce(ParticleLayer* layer, uint32 forceId)
    : CommandAction(CMDID_PARTICLE_FORCE_UPDATE)
{
    this->layer = layer;
    this->forceId = forceId;
}

void CommandUpdateParticleForce::Init(RefPtr<PropertyLine<Vector3>> force,
                                      RefPtr<PropertyLine<float32>> forcesOverLife)
{
    PropertyLineHelper::SetValueLine(this->force, force);
    PropertyLineHelper::SetValueLine(this->forcesOverLife, forcesOverLife);
}

void CommandUpdateParticleForce::Redo()
{
    layer->forces[forceId]->force = force;
    layer->forces[forceId]->forceOverLife = forcesOverLife;
}

//////////////////////////////////////////////////////////////////////////
CommandUpdateParticleDragForce::CommandUpdateParticleDragForce(DAVA::ParticleLayer* layer_, DAVA::uint32 forceId_, ForceParams&& params)
    : CommandAction(CMDID_PARTICLE_DRAG_FORCE_UPDATE)
    , layer(layer_)
    , forceId(forceId_)
    , newParams(params)
{
    DVASSERT(forceId < static_cast<uint32>(layer->GetDragForces().size()));
    DVASSERT(layer != nullptr);
    if (layer != nullptr)
    {
        DAVA::ParticleDragForce* force = layer->GetDragForces()[forceId];
        oldParams.isActive = force->isActive;
        oldParams.useInfinityRange = force->isInfinityRange;
        oldParams.killParticles = force->killParticles;
        oldParams.normalAsReflectionVector = force->normalAsReflectionVector;
        oldParams.pointGravityUseRandomPointsOnSphere = force->pointGravityUseRandomPointsOnSphere;
        oldParams.isGlobal = force->isGlobal;
        oldParams.boxSize = force->boxSize;
        oldParams.radius = force->radius;
        oldParams.forcePower = force->forcePower;
        oldParams.shape = force->shape;
        oldParams.forceName = force->forceName;
        oldParams.timingType = force->timingType;
        oldParams.forcePowerLine = force->forcePowerLine;
        oldParams.turbulenceLine = force->turbulenceLine;
        oldParams.direction = force->direction;
        oldParams.windFrequency = force->windFrequency;
        oldParams.windTurbulence = force->windTurbulence;
        oldParams.pointGravityRadius = force->pointGravityRadius;
        oldParams.planeScale = force->planeScale;
        oldParams.windTurbulenceFrequency = force->windTurbulenceFrequency;
        oldParams.windBias = force->windBias;
        oldParams.backwardTurbulenceProbability = force->backwardTurbulenceProbability;
    }
}

void CommandUpdateParticleDragForce::Redo()
{
    ApplyParams(newParams);
}

void CommandUpdateParticleDragForce::Undo()
{
    ApplyParams(oldParams);
}

void CommandUpdateParticleDragForce::ApplyParams(ForceParams& params)
{
    if (layer != nullptr && forceId < layer->GetDragForces().size())
    {
        DAVA::ParticleDragForce* force = layer->GetDragForces()[forceId];
        force->isActive = params.isActive;
        force->boxSize = params.boxSize;
        force->radius = params.radius;
        force->forcePower = params.forcePower;
        force->shape = params.shape;
        force->isInfinityRange = params.useInfinityRange;
        force->killParticles = params.killParticles;
        force->pointGravityUseRandomPointsOnSphere = params.pointGravityUseRandomPointsOnSphere;
        force->isGlobal = params.isGlobal;
        force->forceName = params.forceName;
        force->timingType = params.timingType;
        force->direction = params.direction;
        force->windFrequency = params.windFrequency;
        force->windTurbulence = params.windTurbulence;
        force->windTurbulenceFrequency = params.windTurbulenceFrequency;
        force->windBias = params.windBias;
        force->backwardTurbulenceProbability = params.backwardTurbulenceProbability;
        force->pointGravityRadius = params.pointGravityRadius;
        force->planeScale = params.planeScale;
        force->normalAsReflectionVector = params.normalAsReflectionVector;
        PropertyLineHelper::SetValueLine(force->forcePowerLine, params.forcePowerLine);
        PropertyLineHelper::SetValueLine(force->turbulenceLine, params.turbulenceLine);
    }
}

CommandAddParticleEmitter::CommandAddParticleEmitter(DAVA::Entity* effect)
    : CommandAction(CMDID_PARTICLE_EMITTER_ADD)
    , effectEntity(effect)
{
}

void CommandAddParticleEmitter::Redo()
{
    if (effectEntity == nullptr)
        return;

    ParticleEffectComponent* effectComponent = GetEffectComponent(effectEntity);
    DVASSERT(effectComponent);
    effectComponent->AddEmitterInstance(ScopedPtr<ParticleEmitter>(new ParticleEmitter()));
}

CommandStartStopParticleEffect::CommandStartStopParticleEffect(DAVA::Entity* effect, bool isStart)
    : CommandAction(CMDID_PARTICLE_EFFECT_START_STOP)
{
    this->isStart = isStart;
    this->effectEntity = effect;
}

void CommandStartStopParticleEffect::Redo()
{
    if (effectEntity == nullptr)
        return;

    ParticleEffectComponent* effectComponent = CastIfEqual<ParticleEffectComponent*>(effectEntity->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
    DVASSERT(effectComponent);

    if (this->isStart)
    {
        effectComponent->Start();
    }
    else
    {
        effectComponent->Stop();
    }
}

DAVA::Entity* CommandStartStopParticleEffect::GetEntity() const
{
    return this->effectEntity;
}

CommandRestartParticleEffect::CommandRestartParticleEffect(DAVA::Entity* effect)
    : CommandAction(CMDID_PARTICLE_EFFECT_RESTART)
{
    this->effectEntity = effect;
}

void CommandRestartParticleEffect::Redo()
{
    if (!effectEntity)
    {
        return;
    }

    ParticleEffectComponent* effectComponent = CastIfEqual<ParticleEffectComponent*>(effectEntity->GetComponent(Component::PARTICLE_EFFECT_COMPONENT));
    DVASSERT(effectComponent);
    effectComponent->Restart();
}

DAVA::Entity* CommandRestartParticleEffect::GetEntity() const
{
    return this->effectEntity;
}

CommandAddParticleEmitterLayer::CommandAddParticleEmitterLayer(DAVA::ParticleEmitterInstance* emitter)
    : CommandAction(CMDID_PARTICLE_EMITTER_LAYER_ADD)
    , instance(emitter)
{
}

CommandAddParticleEmitterLayer::~CommandAddParticleEmitterLayer()
{
    SafeRelease(createdLayer);
}

void CommandAddParticleEmitterLayer::Redo()
{
    if (instance == nullptr)
        return;

    static const float32 LIFETIME_FOR_NEW_PARTICLE_EMITTER = 4.0f;

    createdLayer = new ParticleLayer();
    createdLayer->startTime = 0;
    createdLayer->endTime = LIFETIME_FOR_NEW_PARTICLE_EMITTER;
    createdLayer->life = new PropertyLineValue<float32>(1.0f);
    createdLayer->layerName = ParticlesEditorNodeNameHelper::GetNewLayerName(ResourceEditor::LAYER_NODE_NAME.c_str(), instance->GetEmitter());

    createdLayer->loopEndTime = instance->GetEmitter()->lifeTime;
    instance->GetEmitter()->AddLayer(createdLayer);
}

CommandRemoveParticleEmitterLayer::CommandRemoveParticleEmitterLayer(ParticleEmitterInstance* emitter, ParticleLayer* layer)
    : CommandAction(CMDID_PARTICLE_EMITTER_LAYER_REMOVE)
    , instance(emitter)
    , selectedLayer(layer)
{
}

void CommandRemoveParticleEmitterLayer::Redo()
{
    if ((selectedLayer == nullptr) || (instance == nullptr))
        return;

    instance->GetEmitter()->RemoveLayer(selectedLayer);
}

CommandRemoveParticleEmitter::CommandRemoveParticleEmitter(ParticleEffectComponent* effect, ParticleEmitterInstance* emitter)
    : RECommand(CMDID_PARTICLE_EFFECT_EMITTER_REMOVE)
    , selectedEffect(effect)
    , instance(SafeRetain(emitter))
{
    DVASSERT(selectedEffect != nullptr);
    DVASSERT(instance != nullptr);
}

CommandRemoveParticleEmitter::~CommandRemoveParticleEmitter()
{
    DAVA::SafeRelease(instance);
}

void CommandRemoveParticleEmitter::Redo()
{
    selectedEffect->RemoveEmitterInstance(instance);
}

void CommandRemoveParticleEmitter::Undo()
{
    selectedEffect->AddEmitterInstance(instance);
}

CommandCloneParticleEmitterLayer::CommandCloneParticleEmitterLayer(ParticleEmitterInstance* emitter, ParticleLayer* layer)
    : CommandAction(CMDID_PARTICLE_EMITTER_LAYER_CLONE)
    , instance(emitter)
    , selectedLayer(layer)
{
}

void CommandCloneParticleEmitterLayer::Redo()
{
    if ((selectedLayer == nullptr) || (instance == nullptr))
        return;

    ScopedPtr<ParticleLayer> clonedLayer(selectedLayer->Clone());
    clonedLayer->layerName = selectedLayer->layerName + " Clone";
    instance->GetEmitter()->AddLayer(clonedLayer);
}

CommandAddParticleEmitterForce::CommandAddParticleEmitterForce(ParticleLayer* layer)
    : CommandAction(CMDID_PARTICLE_EMITTER_FORCE_ADD)
    , selectedLayer(layer)
{
}

void CommandAddParticleEmitterForce::Redo()
{
    if (selectedLayer == nullptr)
        return;

    // Add the new Force to the Layer.
    ParticleForce* newForce = new ParticleForce(RefPtr<PropertyLine<Vector3>>(new PropertyLineValue<Vector3>(Vector3(0, 0, 0))), RefPtr<PropertyLine<float32>>(NULL));
    selectedLayer->AddForce(newForce);
    newForce->Release();
}

CommandRemoveParticleEmitterForce::CommandRemoveParticleEmitterForce(ParticleLayer* layer, ParticleForce* force)
    : CommandAction(CMDID_PARTICLE_EMITTER_FORCE_REMOVE)
    , selectedLayer(layer)
    , selectedForce(force)
{
}

void CommandRemoveParticleEmitterForce::Redo()
{
    if ((selectedLayer == nullptr) || (selectedForce == nullptr))
        return;

    selectedLayer->RemoveForce(selectedForce);
}

CommandAddParticleDrag::CommandAddParticleDrag(DAVA::ParticleLayer* layer)
    : CommandAction(CMDID_PARTICLE_EMITTER_DRAG_ADD)
    , selectedLayer(layer)
{
}

void CommandAddParticleDrag::Redo()
{
    AddNewForceToLayer(selectedLayer, ParticleDragForce::eType::DRAG_FORCE);
}


CommandAddParticleLorentzForce::CommandAddParticleLorentzForce(DAVA::ParticleLayer* layer)
    : CommandAction(CMDID_PARTICLE_EMITTER_LORENTZ_FORCE_ADD)
    , selectedLayer(layer)
{
}

void CommandAddParticleLorentzForce::Redo()
{
    AddNewForceToLayer(selectedLayer, ParticleDragForce::eType::LORENTZ_FORCE);
}

CommandAddParticleGravity::CommandAddParticleGravity(DAVA::ParticleLayer* layer)
    : CommandAction(CMDID_PARTICLE_EMITTER_GRAVITY_ADD)
    , selectedLayer(layer)
{
}

void CommandAddParticleGravity::Redo()
{
    AddNewForceToLayer(selectedLayer, ParticleDragForce::eType::GRAVITY);
}

CommandAddParticleWind::CommandAddParticleWind(DAVA::ParticleLayer* layer)
    : CommandAction(CMDID_PARTICLE_EMITTER_WIND_ADD)
    , selectedLayer(layer)
{
}

void CommandAddParticleWind::Redo()
{
    AddNewForceToLayer(selectedLayer, ParticleDragForce::eType::WIND);
}

CommandAddParticlePointGravity::CommandAddParticlePointGravity(DAVA::ParticleLayer* layer)
    : CommandAction(CMDID_PARTICLE_EMITTER_POINT_GRAVITY_ADD)
    , selectedLayer(layer)
{
}

void CommandAddParticlePointGravity::Redo()
{
    AddNewForceToLayer(selectedLayer, ParticleDragForce::eType::POINT_GRAVITY);
}

CommandAddParticlePlaneCollision::CommandAddParticlePlaneCollision(DAVA::ParticleLayer* layer)
    : CommandAction(CMDID_PARTICLE_EMITTER_PLANE_COLLISION_ADD)
    , selectedLayer(layer)
{
}

void CommandAddParticlePlaneCollision::Redo()
{
    AddNewForceToLayer(selectedLayer, ParticleDragForce::eType::PLANE_COLLISION);
}

CommandRemoveParticleDrag::CommandRemoveParticleDrag(ParticleLayer* layer, ParticleDragForce* drag)
    : CommandAction(CMDID_PARTICLE_EMITTER_DRAG_REMOVE)
    , selectedLayer(layer)
    , selectedDrag(drag)
{
}

void CommandRemoveParticleDrag::Redo()
{
    if (selectedLayer == nullptr || selectedDrag == nullptr)
        return;
    selectedLayer->RemoveDrag(selectedDrag);
}

CommandLoadParticleEmitterFromYaml::CommandLoadParticleEmitterFromYaml(ParticleEffectComponent* effect, ParticleEmitterInstance* emitter, const FilePath& path)
    : CommandAction(CMDID_PARTICLE_EMITTER_LOAD_FROM_YAML)
    , selectedEffect(effect)
    , instance(emitter)
    , filePath(path)
{
}

void CommandLoadParticleEmitterFromYaml::Redo()
{
    if ((instance == nullptr) || (selectedEffect == nullptr))
        return;

    auto emitterIndex = selectedEffect->GetEmitterInstanceIndex(instance);
    if (emitterIndex == -1)
        return;

    //TODO: restart effect
    const ParticlesQualitySettings::FilepathSelector* filepathSelector = QualitySettingsSystem::Instance()->GetParticlesQualitySettings().GetOrCreateFilepathSelector();
    FilePath qualityFilepath = filePath;
    if (filepathSelector)
    {
        qualityFilepath = filepathSelector->SelectFilepath(filePath);
    }
    instance->GetEmitter()->LoadFromYaml(qualityFilepath);
    selectedEffect->SetOriginalConfigPath(emitterIndex, filePath);
}

CommandSaveParticleEmitterToYaml::CommandSaveParticleEmitterToYaml(ParticleEffectComponent* effect, ParticleEmitterInstance* emitter, const FilePath& path)
    : CommandAction(CMDID_PARTICLE_EMITTER_SAVE_TO_YAML)
    , selectedEffect(effect)
    , instance(emitter)
    , filePath(path)
{
}

void CommandSaveParticleEmitterToYaml::Redo()
{
    if ((instance == nullptr) || (selectedEffect == nullptr))
        return;

    if (selectedEffect->GetEmitterInstanceIndex(instance) != -1)
    {
        instance->GetEmitter()->SaveToYaml(filePath);
    }
}

CommandLoadInnerParticleEmitterFromYaml::CommandLoadInnerParticleEmitterFromYaml(ParticleEmitterInstance* emitter, const FilePath& path)
    : CommandAction(CMDID_PARTICLE_INNER_EMITTER_LOAD_FROM_YAML)
    , instance(emitter)
    , filePath(path)
{
}

void CommandLoadInnerParticleEmitterFromYaml::Redo()
{
    if (instance == nullptr)
        return;

    //TODO: restart effect
    instance->GetEmitter()->LoadFromYaml(filePath);
}

CommandSaveInnerParticleEmitterToYaml::CommandSaveInnerParticleEmitterToYaml(ParticleEmitterInstance* emitter, const FilePath& path)
    : CommandAction(CMDID_PARTICLE_INNER_EMITTER_SAVE_TO_YAML)
    , instance(emitter)
    , filePath(path)
{
}

void CommandSaveInnerParticleEmitterToYaml::Redo()
{
    if (instance == nullptr)
        return;

    instance->GetEmitter()->SaveToYaml(filePath);
}

CommandCloneParticleDrag::CommandCloneParticleDrag(DAVA::ParticleLayer* layer, DAVA::ParticleDragForce* drag)
    : CommandAction(CMDID_PARTICLE_EMITTER_DRAG_CLONE)
    , selectedLayer(layer)
    , selectedDrag(drag)
{
}

void CommandCloneParticleDrag::Redo()
{
    if ((selectedLayer == nullptr) || (selectedDrag == nullptr))
        return;

    ScopedPtr<ParticleDragForce> clonedForce(selectedDrag->Clone());
    clonedForce->forceName = selectedDrag->forceName + " Clone";
    selectedLayer->AddDrag(clonedForce);
}

void AddNewForceToLayer(ParticleLayer* layer, ParticleDragForce::eType forceType)
{
    if (layer == nullptr)
        return;
    ParticleDragForce* newForce = new ParticleDragForce(layer);
    newForce->type = forceType;

    layer->AddDrag(newForce);
    SafeRelease(newForce);
}

