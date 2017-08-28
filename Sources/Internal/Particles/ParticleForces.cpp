#include "Particles/ParticleForces.h"

#include "Particles/ParticleDragForce.h"
#include "Scene3D/Entity.h"
#include "Math/MathHelpers.h"

namespace DAVA
{
namespace ParticleForces
{
void ApplyDragForce(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife);
void ApplyLorentzForce(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife);
void ApplyPointGravity(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife);
Vector3 GetForceValue(const ParticleDragForce* force, float32 particleOverLife, float32 layerOverLife);

void ApplyForce(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife)
{
    if (!force->isActive)
        return;
    if (force->type == ParticleDragForce::eType::DRAG_FORCE)
    {
        ApplyDragForce(parent, force, effectSpaceVelocity, effectSpacePosition, dt, particleOverLife, layerOverLife);
        return;
    }
    if (force->type == ParticleDragForce::eType::LORENTZ_FORCE)
    {
        ApplyLorentzForce(parent, force, effectSpaceVelocity, effectSpacePosition, dt, particleOverLife, layerOverLife);
    }
}

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

void ApplyDragForce(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife)
{
    if (!force->isInfinityRange)
    {
        if (!IsPositionInForceShape(parent, force, effectSpacePosition))
            return;
    }

    Vector3 forceStrength = GetForceValue(force, particleOverLife, layerOverLife) * dt;

    Vector3 v(Max(0.0f, 1.0f - forceStrength.x), Max(0.0f, 1.0f - forceStrength.y), Max(0.0f, 1.0f - forceStrength.z));
    effectSpaceVelocity *= v;
}

void ApplyLorentzForce(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife)
{
    if (effectSpaceVelocity.DotProduct(force->direction) < EPSILON)
        return;
    if (!force->isInfinityRange)
    {
        if (!IsPositionInForceShape(parent, force, effectSpacePosition))
            return;
    }

    Vector3 forceDir = force->direction.CrossProduct(effectSpaceVelocity);
    forceDir.Normalize();
    effectSpaceVelocity += force->forcePower * forceDir;
}
void ApplyPointGravity(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife)
{
}

Vector3 GetForceValue(const ParticleDragForce* force, float32 particleOverLife, float32 layerOverLife)
{
    if (force->timingType == ParticleDragForce::eTimingType::CONSTANT || force->forcePowerLine == nullptr)
        return force->forcePower;

    if (force->timingType == ParticleDragForce::eTimingType::OVER_PARTICLE_LIFE)
        return force->forcePowerLine->GetValue(particleOverLife);

    if (force->timingType == ParticleDragForce::eTimingType::OVER_LAYER_LIFE)
        return force->forcePowerLine->GetValue(layerOverLife);

    return force->forcePower;
}
}
}