#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class ParticleDragForce;
class Vector3;

namespace ParticleForces
{
void ApplyForce(const ParticleDragForce* force, Vector3& velocity, Vector3& acceleration, float32 dt);
}
}