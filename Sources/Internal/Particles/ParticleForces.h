#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class ParticleDragForce;
class Vector3;
class Entity;

namespace ParticleForces
{
void ApplyForce(Entity* parent, const ParticleDragForce* force, Vector3& velocity, Vector3& acceleration, const Vector3& position, float32 dt);
}
}