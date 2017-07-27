#include "Particles/ParticleDragForce.h"

namespace DAVA
{

DAVA::ParticleDragForce* ParticleDragForce::Clone()
{
    ParticleDragForce* dst = new ParticleDragForce();
    return dst;
}

}