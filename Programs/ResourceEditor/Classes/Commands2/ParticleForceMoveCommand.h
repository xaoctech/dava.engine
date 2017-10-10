#pragma once

#include "Commands2/Base/RECommand.h"
#include "Particles/ParticleLayer.h"
#include "Particles/ParticleForceSimplified.h"

namespace DAVA
{
class ParticleDragForce;
}

class ParticleSimplifiedForceMoveCommand : public RECommand
{
public:
    ParticleSimplifiedForceMoveCommand(DAVA::ParticleForceSimplified* force, DAVA::ParticleLayer* oldLayer, DAVA::ParticleLayer* newLayer);
    ~ParticleSimplifiedForceMoveCommand();

    void Undo() override;
    void Redo() override;

    DAVA::ParticleForceSimplified* force;
    DAVA::ParticleLayer* oldLayer;
    DAVA::ParticleLayer* newLayer;
};

class ParticleDragForceMoveCommand : public RECommand
{
public:
    ParticleDragForceMoveCommand(DAVA::ParticleDragForce* force, DAVA::ParticleLayer* oldLayer, DAVA::ParticleLayer* newLayer);
    ~ParticleDragForceMoveCommand();

    void Undo() override;
    void Redo() override;

    DAVA::ParticleDragForce* force;
    DAVA::ParticleLayer* oldLayer;
    DAVA::ParticleLayer* newLayer;
};
