#include "Particles/ParticleDragForce.h"

#include "Particles/ParticleLayer.h"

namespace DAVA
{

ParticleDragForce::ParticleDragForce(ParticleLayer* parent)
    : parentLayer(parent)
{
    auto emitter = parentLayer->innerEmitter;
}

DAVA::ParticleDragForce* ParticleDragForce::Clone()
{
    ParticleDragForce* dst = new ParticleDragForce(parentLayer);
    return dst;
}

}