#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class ParticleDragForce;
class Vector3;
class Entity;

namespace ParticleForces
{
/** Modifies effect space velocity. */
void ApplyForce(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife);
}
}