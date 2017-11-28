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
void ApplyForce(const ParticleForce* force, Vector3& velocity, Vector3& position, float32 dt, float32 particleOverLife, float32 layerOverLife, const Vector3& down, Particle* particle, const Vector3& prevPosition, const Vector3& forcePosition);
}
}