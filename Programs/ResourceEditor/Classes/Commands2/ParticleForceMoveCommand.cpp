#include "Commands2/ParticleForceMoveCommand.h"
#include "Commands2/RECommandIDs.h"

#include <Particles/ParticleDragForce.h>

ParticleSimplifiedForceMoveCommand::ParticleSimplifiedForceMoveCommand(DAVA::ParticleForceSimplified* _force, DAVA::ParticleLayer* _oldLayer, DAVA::ParticleLayer* _newLayer)
    : RECommand(CMDID_PARTICLE_SIMPLIFIED_FORCE_MOVE, "Move particle simplified force")
    , force(_force)
    , oldLayer(_oldLayer)
    , newLayer(_newLayer)
{
    SafeRetain(force);
}

ParticleSimplifiedForceMoveCommand::~ParticleSimplifiedForceMoveCommand()
{
    SafeRelease(force);
}

void ParticleSimplifiedForceMoveCommand::Undo()
{
    if (NULL != force)
    {
        if (NULL != newLayer)
        {
            newLayer->RemoveSimplifiedForce(force);
        }

        if (NULL != oldLayer)
        {
            oldLayer->AddSimplifiedForce(force);
        }
    }
}

void ParticleSimplifiedForceMoveCommand::Redo()
{
    if (NULL != force)
    {
        if (NULL != oldLayer)
        {
            oldLayer->RemoveSimplifiedForce(force);
        }

        if (NULL != newLayer)
        {
            newLayer->AddSimplifiedForce(force);
        }
    }
}

ParticleDragForceMoveCommand::ParticleDragForceMoveCommand(DAVA::ParticleDragForce* force, DAVA::ParticleLayer* oldLayer, DAVA::ParticleLayer* newLayer)
    : RECommand(CMDID_PARTICLE_DRAG_FORCE_MOVE, "Move particle drag force")
    , force(force)
    , oldLayer(oldLayer)
    , newLayer(newLayer)
{
    SafeRetain(force);
}

ParticleDragForceMoveCommand::~ParticleDragForceMoveCommand()
{
    SafeRelease(force);
}

void ParticleDragForceMoveCommand::Undo()
{
    if (force != nullptr)
    {
        if (newLayer != nullptr)
        {
            newLayer->RemoveDrag(force);
        }

        if (oldLayer != nullptr)
        {
            oldLayer->AddDrag(force);
        }
    }
}

void ParticleDragForceMoveCommand::Redo()
{
    if (force != nullptr)
    {
        if (oldLayer != nullptr)
        {
            oldLayer->RemoveDrag(force);
        }

        if (newLayer != nullptr)
        {
            newLayer->AddDrag(force);
        }
    }
}
