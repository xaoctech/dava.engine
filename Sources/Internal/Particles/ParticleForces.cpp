#include "Particles/ParticleForces.h"

#include "Particles/ParticleDragForce.h"

namespace DAVA
{
namespace ParticleForces
{
void ApplyDragForce(const ParticleDragForce* force, Vector3& velocity, Vector3& acceleration, float32 dt);

void ApplyForce(const ParticleDragForce* force, Vector3& velocity, Vector3& acceleration, float32 dt)
{
    if (force->type == ParticleDragForce::eType::DRAG_FORCE)
    {
        ApplyDragForce(force, velocity, acceleration, dt);
        return;
    }
}

void ApplyDragForce(const ParticleDragForce* force, Vector3& velocity, Vector3& acceleration, float32 dt)
{
    const Vector3& forceStrength = force->forcePower;
    Vector3 velSqr = velocity * velocity;
    Vector3 dragForce = -forceStrength * velSqr;
    //acceleration += dragForce;
    float32 vLen = velocity.Length();
    Vector3 v(Max(0.0f, 1.0f - forceStrength.x), Max(0.0f, 1.0f - forceStrength.y), Max(0.0f, 1.0f - forceStrength.z));
    velocity *= v;
}

}
}