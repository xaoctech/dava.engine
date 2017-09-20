#include "Particles/ParticleDragForce.h"

#include "Reflection/ReflectionRegistrator.h"

namespace DAVA
{
DAVA_VIRTUAL_REFLECTION_IMPL(ParticleDragForce)
{
    ReflectionRegistrator<ParticleDragForce>::Begin()
        .End();
}

ParticleDragForce::ParticleDragForce(ParticleLayer* parent)
    : parentLayer(parent)
{
}

ParticleDragForce* ParticleDragForce::Clone()
{
    ParticleDragForce* dst = new ParticleDragForce(parentLayer);
    if (forcePowerLine != nullptr)
    {
        dst->forcePowerLine = forcePowerLine->Clone();
        dst->forcePowerLine->Release();
    }
    if (turbulenceLine != nullptr)
    {
        dst->turbulenceLine = turbulenceLine->Clone();
        dst->turbulenceLine->Release();
    }
    dst->direction = direction;
    dst->isActive = isActive;
    dst->timingType = timingType;
    dst->forceName = forceName;
    dst->shape = shape;
    dst->type = type;
    dst->parentLayer = parentLayer;
    dst->position = position;
    dst->rotation = rotation;
    dst->isInfinityRange = isInfinityRange;
    dst->boxSize = boxSize;
    dst->forcePower = forcePower;
    dst->radius = radius;
    dst->windFrequency = windFrequency;
    dst->windTurbulenceFrequency = windTurbulenceFrequency;
    dst->windTurbulence = windTurbulence;
    dst->backwardTurbulenceProbability = backwardTurbulenceProbability;
    dst->windBias = windBias;

    return dst;
}

void ParticleDragForce::GetModifableLines(List<ModifiablePropertyLineBase*>& modifiables)
{
    PropertyLineHelper::AddIfModifiable(forcePowerLine.Get(), modifiables);
    PropertyLineHelper::AddIfModifiable(turbulenceLine.Get(), modifiables);
}
}