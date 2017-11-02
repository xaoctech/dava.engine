#pragma once

#include "REPlatform/Commands/RECommand.h"

#include <Reflection/Reflection.h>

namespace DAVA
{
struct ParticleLayer;
class ParticleEmitterInstance;
class ParticleLayerMoveCommand : public RECommand
{
public:
    ParticleLayerMoveCommand(ParticleEmitterInstance* oldEmitter, ParticleLayer* layer, ParticleEmitterInstance* newEmitter, ParticleLayer* newBefore = NULL);
    ~ParticleLayerMoveCommand();

    void Undo() override;
    void Redo() override;

private:
    ParticleLayer* layer = nullptr;
    ParticleEmitterInstance* oldEmitter = nullptr;
    ParticleLayer* oldBefore = nullptr;
    ParticleEmitterInstance* newEmitter = nullptr;
    ParticleLayer* newBefore = nullptr;

    DAVA_VIRTUAL_REFLECTION(ParticleLayerMoveCommand, RECommand);
};
} // namespace DAVA
