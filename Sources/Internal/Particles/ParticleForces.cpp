#include "Particles/ParticleForces.h"

#include "Particles/ParticleDragForce.h"
#include "Scene3D/Entity.h"
#include "Math/MathHelpers.h"

namespace DAVA
{
namespace ParticleForces
{
void ApplyDragForce(const Entity* parent, const ParticleDragForce* force, Vector3& velocity, Vector3& acceleration, const Vector3& position, float32 dt);

void ApplyForce(Entity* parent, const ParticleDragForce* force, Vector3& velocity, Vector3& acceleration, const Vector3& position, float32 dt)
{
    if (force->type == ParticleDragForce::eType::DRAG_FORCE)
    {
        ApplyDragForce(parent, force, velocity, acceleration, position, dt);
        return;
    }
}

void ApplyDragForce(const Entity* parent, const ParticleDragForce* force, Vector3& velocity, Vector3& acceleration, const Vector3& position, float32 dt)
{
    if (!force->infinityRange)
    {
        Vector3 fPos = force->position;
        Vector3 transformedPos = TransformPerserveLength(fPos, DAVA::Matrix3(parent->GetWorldTransform()));
        Vector3 center = parent->GetWorldTransform().GetTranslationVector() + transformedPos;
        if (force->shape == ParticleDragForce::eShape::BOX)
        {
            Vector3 size = force->boxSize * 0.5f;
            Vector3 p1 = center - size;
            Vector3 p2 = center + size;
            AABBox3 box(p1, p2);

            if (!box.IsInside(position))
                return;
        }
        else if (force->shape == ParticleDragForce::eShape::SPHERE)
        {
            float32 distSqr = (center - position).SquareLength();
            if (distSqr > force->radius * force->radius)
                return;
        }
    }

    const Vector3& forceStrength = force->forcePower * dt;
    Vector3 velSqr = velocity * velocity;
    Vector3 dragForce = -forceStrength * velSqr * dt;
    //acceleration += dragForce;
    float32 vLen = velocity.Length();
    Vector3 v(Max(0.0f, 1.0f - forceStrength.x), Max(0.0f, 1.0f - forceStrength.y), Max(0.0f, 1.0f - forceStrength.z));
    velocity *= v;
}

}
}