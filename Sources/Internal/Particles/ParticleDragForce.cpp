#include "Particles/ParticleDragForce.h"

#include "Particles/ParticleLayer.h"

namespace DAVA
{

ParticleDragForce::ParticleDragForce(ParticleLayer* parent)
    : parentLayer(parent)
{
}

ParticleDragForce* ParticleDragForce::Clone()
{
    ParticleDragForce* dst = new ParticleDragForce(parentLayer);
    dst->parentLayer = parentLayer;
    dst->position = position;
    dst->rotation = rotation;
    dst->infinityRange = infinityRange;
    return dst;
}

}