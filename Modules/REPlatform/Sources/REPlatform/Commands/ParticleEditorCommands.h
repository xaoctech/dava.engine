#pragma once

#include "REPlatform/Commands/CommandAction.h"

#include <Base/BaseTypes.h>
#include <Base/RefPtr.h>
#include <FileSystem/FilePath.h>
#include <Math/Vector.h>
#include <Particles/ParticleEmitter.h>
#include <Particles/ParticleLayer.h>
#include <Particles/ParticlePropertyLine.h>

#include <Reflection/Reflection.h>

namespace DAVA
{
class Entity;
class ParticleEmitterInstance;
class ParticleEffectComponent;
class ParticleForce;

class CommandAddParticleEmitter : public CommandAction
{
public:
    CommandAddParticleEmitter(Entity* effect);
    void Redo() override;

protected:
    Entity* effectEntity = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandAddParticleEmitter, CommandAction);
};

// Start/stop/restart Particle Effect.
class CommandStartStopParticleEffect : public CommandAction
{
public:
    CommandStartStopParticleEffect(Entity* effect, bool isStart);

    Entity* GetEntity() const;
    void Redo() override;

    bool GetStarted() const
    {
        return isStart;
    };

    bool IsClean() const override
    {
        return true;
    }

protected:
    Entity* effectEntity;
    bool isStart = false;

    DAVA_VIRTUAL_REFLECTION(CommandStartStopParticleEffect, CommandAction);
};

class CommandRestartParticleEffect : public CommandAction
{
public:
    CommandRestartParticleEffect(Entity* effect);

    Entity* GetEntity() const;
    void Redo() override;

    bool IsClean() const override
    {
        return true;
    }

protected:
    Entity* effectEntity = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandRestartParticleEffect, CommandAction);
};

// Add new layer to Particle Emitter.
class CommandAddParticleEmitterLayer : public CommandAction
{
public:
    CommandAddParticleEmitterLayer(ParticleEmitterInstance* emitter);
    ~CommandAddParticleEmitterLayer();

    void Redo() override;

    ParticleLayer* GetCreatedLayer() const
    {
        return createdLayer;
    };
    ParticleEmitterInstance* GetParentEmitter() const
    {
        return instance;
    }

protected:
    ParticleEmitterInstance* instance = nullptr;
    ParticleLayer* createdLayer = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandAddParticleEmitterLayer, CommandAction);
};

// Remove a layer from Particle Emitter.
class CommandRemoveParticleEmitterLayer : public CommandAction
{
public:
    CommandRemoveParticleEmitterLayer(ParticleEmitterInstance* emitter, ParticleLayer* layer);
    void Redo() override;

protected:
    ParticleEmitterInstance* instance = nullptr;
    ParticleLayer* selectedLayer = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandRemoveParticleEmitterLayer, CommandAction);
};

class CommandRemoveParticleEmitter : public RECommand
{
public:
    CommandRemoveParticleEmitter(ParticleEffectComponent* effect, ParticleEmitterInstance* emitter);
    ~CommandRemoveParticleEmitter();

    void Redo() override;
    void Undo() override;

    ParticleEffectComponent* GetEffect() const;
    ParticleEmitterInstance* GetEmitterInstance() const
    {
        return instance;
    }

protected:
    ParticleEffectComponent* selectedEffect = nullptr;
    ParticleEmitterInstance* instance = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandRemoveParticleEmitter, RECommand);
};

inline ParticleEffectComponent* CommandRemoveParticleEmitter::GetEffect() const
{
    return selectedEffect;
}

// Clone a layer inside Particle Emitter.
class CommandCloneParticleEmitterLayer : public CommandAction
{
public:
    CommandCloneParticleEmitterLayer(ParticleEmitterInstance* emitter, ParticleLayer* layer);
    void Redo() override;

protected:
    ParticleEmitterInstance* instance = nullptr;
    ParticleLayer* selectedLayer = nullptr;

    DAVA_VIRTUAL_REFLECTION(CommandCloneParticleEmitterLayer, CommandAction);
};

// Add new force to Particle Emitter layer.
class CommandAddParticleEmitterForce : public CommandAction
{
public:
    CommandAddParticleEmitterForce(ParticleLayer* layer);
    void Redo() override;

protected:
    ParticleLayer* selectedLayer = nullptr;
    DAVA_VIRTUAL_REFLECTION(CommandAddParticleEmitterForce, CommandAction);
};

// Remove a force from Particle Emitter layer.
class CommandRemoveParticleEmitterForce : public CommandAction
{
public:
    CommandRemoveParticleEmitterForce(ParticleLayer* layer, ParticleForce* force);
    void Redo() override;

protected:
    ParticleLayer* selectedLayer = nullptr;
    ParticleForce* selectedForce = nullptr;
    DAVA_VIRTUAL_REFLECTION(CommandRemoveParticleEmitterForce, CommandAction);
};

class CommandUpdateEffect : public CommandAction
{
public:
    CommandUpdateEffect(ParticleEffectComponent* particleEffect);
    void Init(float32 playbackSpeed);
    void Redo() override;

protected:
    ParticleEffectComponent* particleEffect = nullptr;
    float32 playbackSpeed = 1.0f;

    DAVA_VIRTUAL_REFLECTION(CommandUpdateEffect, CommandAction);
};

class CommandUpdateEmitter : public CommandAction
{
public:
    CommandUpdateEmitter(ParticleEmitterInstance* emitter);

    void Init(const FastName& name,
              ParticleEmitter::eType emitterType,
              RefPtr<PropertyLine<float32>> emissionRange,
              RefPtr<PropertyLine<Vector3>> emissionVector,
              RefPtr<PropertyLine<float32>> radius,
              RefPtr<PropertyLine<float32>> emissionAngle,
              RefPtr<PropertyLine<float32>> emissionAngleVariation,
              RefPtr<PropertyLine<Color>> colorOverLife,
              RefPtr<PropertyLine<Vector3>> size,
              float32 life,
              bool isShortEffect);

    ParticleEmitterInstance* GetEmitterInstance() const
    {
        return instance;
    }

    void Redo() override;

protected:
    FastName name;
    ParticleEmitterInstance* instance = nullptr;

    ParticleEmitter::eType emitterType;
    RefPtr<PropertyLine<float32>> emissionRange;
    RefPtr<PropertyLine<float32>> emissionAngle;
    RefPtr<PropertyLine<float32>> emissionAngleVariation;
    RefPtr<PropertyLine<Vector3>> emissionVector;
    RefPtr<PropertyLine<float32>> radius;
    RefPtr<PropertyLine<Color>> colorOverLife;
    RefPtr<PropertyLine<Vector3>> size;
    float32 life = 0.0f;
    bool isShortEffect = false;

    DAVA_VIRTUAL_REFLECTION(CommandUpdateEmitter, CommandAction);
};

class CommandUpdateParticleLayerBase : public CommandAction
{
public:
    CommandUpdateParticleLayerBase()
        : CommandAction()
    {
    }

    ParticleLayer* GetLayer() const
    {
        return layer;
    };

protected:
    ParticleLayer* layer;

    DAVA_VIRTUAL_REFLECTION(CommandUpdateParticleLayerBase, CommandAction);
};

class CommandUpdateParticleLayer : public CommandUpdateParticleLayerBase
{
public:
    CommandUpdateParticleLayer(ParticleEmitterInstance* emitter, ParticleLayer* layer);
    void Init(const String& layerName,
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
              float32 pivotPointY);

    void Redo() override;

protected:
    ParticleEmitterInstance* emitter = nullptr;

    String layerName;
    ParticleLayer::eType layerType;
    ParticleLayer::eDegradeStrategy degradeStrategy;
    bool isDisabled;
    bool isLong;
    float32 scaleVelocityBase;
    float32 scaleVelocityFactor;
    bool inheritPosition;
    bool isLooped;
    int32 particleOrientation;
    RefPtr<PropertyLine<float32>> life;
    RefPtr<PropertyLine<float32>> lifeVariation;
    RefPtr<PropertyLine<float32>> number;
    RefPtr<PropertyLine<float32>> numberVariation;
    RefPtr<PropertyLine<Vector2>> size;
    RefPtr<PropertyLine<Vector2>> sizeVariation;
    RefPtr<PropertyLine<Vector2>> sizeOverLife;
    RefPtr<PropertyLine<float32>> velocity;
    RefPtr<PropertyLine<float32>> velocityVariation;
    RefPtr<PropertyLine<float32>> velocityOverLife;
    RefPtr<PropertyLine<float32>> spin;
    RefPtr<PropertyLine<float32>> spinVariation;
    RefPtr<PropertyLine<float32>> spinOverLife;
    bool randomSpinDirection;

    RefPtr<PropertyLine<Color>> colorRandom;
    RefPtr<PropertyLine<float32>> alphaOverLife;
    RefPtr<PropertyLine<Color>> colorOverLife;
    RefPtr<PropertyLine<float32>> angle;
    RefPtr<PropertyLine<float32>> angleVariation;

    float32 startTime;
    float32 endTime;
    float32 deltaTime;
    float32 deltaVariation;
    float32 loopEndTime;
    float32 loopVariation;
    bool frameOverLifeEnabled;
    float32 frameOverLifeFPS;
    bool randomFrameOnStart;
    bool loopSpriteAnimation;
    RefPtr<PropertyLine<float32>> animSpeedOverLife;

    float32 pivotPointX;
    float32 pivotPointY;

    DAVA_VIRTUAL_REFLECTION(CommandUpdateParticleLayer, CommandUpdateParticleLayerBase);
};

class CommandUpdateParticleLayerTime : public CommandUpdateParticleLayerBase
{
public:
    CommandUpdateParticleLayerTime(ParticleLayer* layer);
    void Init(float32 startTime, float32 endTime);

    void Redo() override;

protected:
    float32 startTime;
    float32 endTime;

    DAVA_VIRTUAL_REFLECTION(CommandUpdateParticleLayerTime, CommandUpdateParticleLayerBase);
};

class CommandUpdateParticleLayerEnabled : public CommandUpdateParticleLayerBase
{
public:
    CommandUpdateParticleLayerEnabled(ParticleLayer* layer, bool isEnabled);
    void Redo() override;

protected:
    bool isEnabled;
    DAVA_VIRTUAL_REFLECTION(CommandUpdateParticleLayerEnabled, CommandUpdateParticleLayerBase);
};

class CommandUpdateParticleLayerLods : public CommandUpdateParticleLayerBase
{
public:
    CommandUpdateParticleLayerLods(ParticleLayer* layer, const Vector<bool>& lods);
    void Redo() override;

protected:
    Vector<bool> lods;
    DAVA_VIRTUAL_REFLECTION(CommandUpdateParticleLayerLods, CommandUpdateParticleLayerBase);
};

class CommandUpdateParticleForce : public CommandAction
{
public:
    CommandUpdateParticleForce(ParticleLayer* layer, uint32 forceId);

    void Init(RefPtr<PropertyLine<Vector3>> force,
              RefPtr<PropertyLine<float32>> forcesOverLife);

    void Redo() override;

    ParticleLayer* GetLayer() const
    {
        return layer;
    };
    uint32 GetForceIndex() const
    {
        return forceId;
    };

protected:
    ParticleLayer* layer;
    uint32 forceId;

    RefPtr<PropertyLine<Vector3>> force;
    RefPtr<PropertyLine<float32>> forcesOverLife;

    DAVA_VIRTUAL_REFLECTION(CommandUpdateParticleForce, CommandAction);
};

// Load/save Particle Emitter Node.
class CommandLoadParticleEmitterFromYaml : public CommandAction
{
public:
    CommandLoadParticleEmitterFromYaml(ParticleEffectComponent* effect, ParticleEmitterInstance* emitter, const FilePath& path);

    void Redo() override;

    ParticleEmitterInstance* GetEmitterInstance() const
    {
        return instance;
    }

protected:
    ParticleEffectComponent* selectedEffect = nullptr;
    ParticleEmitterInstance* instance = nullptr;
    FilePath filePath;

    DAVA_VIRTUAL_REFLECTION(CommandLoadParticleEmitterFromYaml, CommandAction);
};

class CommandSaveParticleEmitterToYaml : public CommandAction
{
public:
    CommandSaveParticleEmitterToYaml(ParticleEffectComponent* effect, ParticleEmitterInstance* emitter, const FilePath& path);

    void Redo() override;

    ParticleEmitterInstance* GetEmitterInstance() const
    {
        return instance;
    }

protected:
    ParticleEffectComponent* selectedEffect = nullptr;
    ParticleEmitterInstance* instance = nullptr;
    FilePath filePath;

    DAVA_VIRTUAL_REFLECTION(CommandSaveParticleEmitterToYaml, CommandAction);
};

// Load/save Particle Inner Emitter Node.
class CommandLoadInnerParticleEmitterFromYaml : public CommandAction
{
public:
    CommandLoadInnerParticleEmitterFromYaml(ParticleEmitterInstance* emitter, const FilePath& path);

    void Redo() override;

    ParticleEmitterInstance* GetEmitterInstance() const
    {
        return instance;
    }

protected:
    ParticleEmitterInstance* instance = nullptr;
    FilePath filePath;

    DAVA_VIRTUAL_REFLECTION(CommandLoadInnerParticleEmitterFromYaml, CommandAction);
};

class CommandSaveInnerParticleEmitterToYaml : public CommandAction
{
public:
    CommandSaveInnerParticleEmitterToYaml(ParticleEmitterInstance* emitter, const FilePath& path);
    void Redo() override;

    ParticleEmitterInstance* GetEmitterInstance() const
    {
        return instance;
    }

protected:
    ParticleEmitterInstance* instance = nullptr;
    FilePath filePath;

    DAVA_VIRTUAL_REFLECTION(CommandSaveInnerParticleEmitterToYaml, CommandAction);
};
} // namespace DAVA
