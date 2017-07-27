#include "Commands2/ParticleForceMoveCommand.h"
#include "Commands2/RECommandIDs.h"

#include <Particles/ParticleDrag.h>

ParticleForceMoveCommand::ParticleForceMoveCommand(DAVA::ParticleForce* _force, DAVA::ParticleLayer* _oldLayer, DAVA::ParticleLayer* _newLayer)
    : RECommand(CMDID_PARTICLE_FORCE_MOVE, "Move particle force")
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
    if (NULL != force)
    {
        if (NULL != newLayer)
        {
            newLayer->RemoveForce(force);
        }

        if (NULL != oldLayer)
        {
            oldLayer->AddForce(force);
        }
    }
}

void ParticleForceMoveCommand::Redo()
{
    if (NULL != force)
    {
        if (NULL != oldLayer)
        {
            oldLayer->RemoveForce(force);
        }

        if (NULL != newLayer)
        {
            newLayer->AddForce(force);
        }
    }
}

ParticleDragForceMoveCommand::ParticleDragForceMoveCommand(DAVA::ParticleDrag* force, DAVA::ParticleLayer* oldLayer, DAVA::ParticleLayer* newLayer) : RECommand(CMDID_PARTICLE_DRAG_FORCE_MOVE, "Move particle drag force")
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
