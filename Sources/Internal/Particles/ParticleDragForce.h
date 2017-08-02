#pragma once

#include "Base/BaseObject.h"

namespace DAVA
{
struct ParticleLayer;

class ParticleDragForce : public BaseObject
{
public:
    ParticleDragForce(ParticleLayer* parent);

    ParticleDragForce* Clone();
public:
    INTROSPECTION_EXTEND(ParticleDragForce, BaseObject, nullptr)

private:
    ParticleLayer* parentLayer = nullptr;
};
}
