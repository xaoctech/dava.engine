#include "Particles/ParticleEmitterInstance.h"

#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(ParticleEmitterInstance)
{
    ReflectionRegistrator<ParticleEmitterInstance>::Begin()
    .End();
}

ParticleEmitterInstance::ParticleEmitterInstance(ParticleEffectComponent* owner_, bool isInner)
    : owner(owner_)
    , isInnerEmitter(isInner)
{
}

ParticleEmitterInstance::ParticleEmitterInstance(ParticleEffectComponent* owner_, ParticleEmitter* emitter_, bool isInner)
    : owner(owner_)
    , emitter(SafeRetain(emitter_))
    , isInnerEmitter(isInner)
{
}

ParticleEmitterInstance* ParticleEmitterInstance::Clone() const
{
    ScopedPtr<ParticleEmitter> clonedEmitter(emitter->Clone());
    ParticleEmitterInstance* result = new ParticleEmitterInstance(owner, clonedEmitter.get());
    result->SetFilePath(GetFilePath());
    result->SetSpawnPosition(GetSpawnPosition());
    result->isInnerEmitter = isInnerEmitter;
    return result;
}
}
