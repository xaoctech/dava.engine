#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Particles/ParticleLayer.h>
#include <Particles/ParticleForceSimplified.h>
#include <Reflection/Reflection.h>

namespace DAVA
{
class ParticleForceSimplifiedRemoveCommand : public RECommand
{
public:
    ParticleForceSimplifiedRemoveCommand(ParticleForceSimplified* force, ParticleLayer* layer);
    ~ParticleForceSimplifiedRemoveCommand();

    void Undo() override;
    void Redo() override;

    ParticleForceSimplified* force;
    ParticleLayer* layer;

private:
    DAVA_VIRTUAL_REFLECTION(ParticleForceSimplifiedRemoveCommand, RECommand);
};
} // namespace DAVA
