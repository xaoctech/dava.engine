#pragma once

#include "Commands2/Base/RECommand.h"
#include "Particles/ParticleLayer.h"
#include "Particles/ParticleForce.h"

namespace DAVA
{
class ParticleDrag;
}

class ParticleForceMoveCommand : public RECommand
{
public:
    ParticleForceMoveCommand(DAVA::ParticleForce* force, DAVA::ParticleLayer* oldLayer, DAVA::ParticleLayer* newLayer);
    ~ParticleForceMoveCommand();

    void Undo() override;
    void Redo() override;

    DAVA::ParticleForce* force;
    DAVA::ParticleLayer* oldLayer;
    DAVA::ParticleLayer* newLayer;
};

class ParticleDragForceMoveCommand : public RECommand
{
public:
    ParticleDragForceMoveCommand(DAVA::ParticleDrag* force, DAVA::ParticleLayer* oldLayer, DAVA::ParticleLayer* newLayer);
    ~ParticleDragForceMoveCommand();

    void Undo() override;
    void Redo() override;

    DAVA::ParticleDrag* force;
    DAVA::ParticleLayer* oldLayer;
    DAVA::ParticleLayer* newLayer;
};

