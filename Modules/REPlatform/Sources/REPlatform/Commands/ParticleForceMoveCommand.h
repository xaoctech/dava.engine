#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
struct ParticleLayer;
class ParticleForce;
class ParticleForceMoveCommand : public RECommand
{
public:
    ParticleForceMoveCommand(ParticleForce* force, ParticleLayer* oldLayer, ParticleLayer* newLayer);
    ~ParticleForceMoveCommand();

    void Undo() override;
    void Redo() override;

    ParticleForce* force;
    ParticleLayer* oldLayer;
    ParticleLayer* newLayer;

private:
    DAVA_VIRTUAL_REFLECTION(ParticleForceMoveCommand, RECommand);
};
} // namespace DAVA
