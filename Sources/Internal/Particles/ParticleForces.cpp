#include "Particles/ParticleForces.h"

#include "Particles/ParticleDragForce.h"
#include "Scene3D/Entity.h"
#include "Math/MathHelpers.h"

namespace DAVA
{
namespace ParticleForces
{
void ApplyDragForce(Entity* parent, const ParticleDragForce* force, Vector3& velocity, Vector3& acceleration, Vector3 position, float32 dt);
void ApplyLorentzForce(Entity* parent, const ParticleDragForce* force, Vector3& velocity, Vector3& acceleration, Vector3 position, float32 dt);
void ApplyPointGravity(Entity* parent, const ParticleDragForce* force, Vector3& velocity, Vector3& acceleration, Vector3 position, float32 dt);

void ApplyForce(Entity* parent, const ParticleDragForce* force, Vector3& velocity, Vector3& acceleration, const Vector3& position, float32 dt)
{
    if (force->type == ParticleDragForce::eType::DRAG_FORCE)
    {
        ApplyDragForce(parent, force, velocity, acceleration, position, dt);
        return;
    }
}

// POsition here in effect space directly!!!!!!!!!!!!!!
bool IsPositionInForceShape(const Entity* parent, const ParticleDragForce* force, const Vector3& effectSpacePosition)
{
    Vector3 center = force->position;
    if (force->shape == ParticleDragForce::eShape::BOX)
    {
        Vector3 size = force->boxSize * 0.5f;
        Vector3 p1 = center - size;
        Vector3 p2 = center + size;
        AABBox3 box(p1, p2);

        if (box.IsInside(effectSpacePosition))
            return true;
    }
    else if (force->shape == ParticleDragForce::eShape::SPHERE)
    {
        float32 distSqr = (center - effectSpacePosition).SquareLength();
        if (distSqr <= force->radius * force->radius)
            return true;
    }
    return false;
}

void ApplyDragForce(Entity* parent, const ParticleDragForce* force, Vector3& velocityW, Vector3& acceleration, Vector3 position, float32 dt)
{
    const Matrix4& world = parent->GetWorldTransform();

    Matrix4 local;
    bool sucess = world.GetInverse(local); // todo: in transform
    if (!force->infinityRange)
    {
        Vector3 positionInEffectsSpace = position * local;
        if (!IsPositionInForceShape(parent, force, positionInEffectsSpace))
            return;
    }

    Vector3 forceStrength = force->forcePower* dt;
    Vector3 velocity = velocityW;
    velocity = velocity * Matrix3(local); // TODO: should be simpler.

    Vector3 v(Max(0.0f, 1.0f - forceStrength.x), Max(0.0f, 1.0f - forceStrength.y), Max(0.0f, 1.0f - forceStrength.z)); // todo calculate all in effect space
    velocity *= v;
    velocityW = velocity * Matrix3(world);
}


void ApplyLorentzForce(Entity* parent, const ParticleDragForce* force, Vector3& velocity, Vector3& acceleration, Vector3 position, float32 dt)
{}
void ApplyPointGravity(Entity* parent, const ParticleDragForce* force, Vector3& velocity, Vector3& acceleration, Vector3 position, float32 dt)
{}

}
}