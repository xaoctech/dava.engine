#pragma once

#include "Base/BaseTypes.h"

namespace DAVA
{
class ParticleForce;
class Vector3;
class Entity;
struct Particle;

namespace ParticleForces
{
void Init();
/** Modifies effect space velocity. */
void ApplyForce(Entity* parent, const ParticleForce* force, Vector3& effectSpaceVelocity, Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife, const Vector3& effectSpaceDown, Particle* particle, const Vector3& prevEffectSpacePosition);
}
}