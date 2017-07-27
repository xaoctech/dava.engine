#include "Particles/ParticleDrag.h"

namespace DAVA
{

DAVA::ParticleDrag* ParticleDrag::Clone()
{
    ParticleDrag* dst = new ParticleDrag();
    return dst;
}

}