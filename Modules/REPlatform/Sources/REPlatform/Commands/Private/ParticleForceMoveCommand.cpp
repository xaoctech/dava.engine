#include "REPlatform/Commands/ParticleForceMoveCommand.h"

#include <Particles/ParticleLayer.h>
#include <Particles/ParticleForce.h>
#include <Reflection/ReflectionRegistrator.h>

namespace DAVA
{
ParticleForceMoveCommand::ParticleForceMoveCommand(ParticleForce* _force, ParticleLayer* _oldLayer, ParticleLayer* _newLayer)
    : RECommand("Move particle force")
    , force(_force)
    , oldLayer(_oldLayer)
    , newLayer(_newLayer)
{
    SafeRetain(force);
}

ParticleForceMoveCommand::~ParticleForceMoveCommand()
{
    SafeRelease(force);
}

void ParticleForceMoveCommand::Undo()
{
    if (nullptr != force)
    {
        if (nullptr != newLayer)
        {
            newLayer->RemoveForce(force);
        }

        if (nullptr != oldLayer)
        {
            oldLayer->AddForce(force);
        }
    }
}

void ParticleForceMoveCommand::Redo()
{
    if (nullptr != force)
    {
        if (nullptr != oldLayer)
        {
            oldLayer->RemoveForce(force);
        }

        if (nullptr != newLayer)
        {
            newLayer->AddForce(force);
        }
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(ParticleForceMoveCommand)
{
    ReflectionRegistrator<ParticleForceMoveCommand>::Begin()
    .End();
}
} // namespace DAVA
