#include "Particles/ParticleDragForce.h"

#include "Particles/ParticleLayer.h"

namespace DAVA
{

ParticleDragForce::ParticleDragForce(ParticleLayer* parent)
    : parentLayer(parent)
{
    boxSize = { 1.03f, 23.13f, 3.0f };
}

DAVA::ParticleDragForce* ParticleDragForce::Clone()
{
    ParticleDragForce* dst = new ParticleDragForce(parentLayer);
    dst->parentLayer = parentLayer;
    dst->position = position;
    dst->rotation = rotation;
    dst->infinityRange = infinityRange;
    return dst;
}

}