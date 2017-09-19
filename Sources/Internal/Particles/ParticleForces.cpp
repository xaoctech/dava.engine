#include "Particles/ParticleForces.h"

#include "Particles/ParticleDragForce.h"
#include "Scene3D/Entity.h"
#include "Math/MathHelpers.h"
#include "Math/Noise.h"
#include "Particles/Particle.h"

namespace DAVA
{
namespace ParticleForces
{
const int32 noiseWidth = 128;
const int32 noiseHeight = 128;
Array<Array<Vector3, noiseWidth>, noiseHeight> noise;

void GenerateNoise()
{
    float32 xFactor = 1.0f / (noiseWidth - 1.0f);
    float32 yFactor = 1.0f / (noiseHeight - 1.0f);
    for (int32 i = 0; i < noiseWidth; ++i)
    {
        Vector2 q(i * xFactor, 0);
        for (int32 j = 0; j < noiseHeight; ++j)
        {
            q.y = j * yFactor;
            noise[i][j] = Generate4OctavesPerlin(q);
        }
    }
}


const float32 windPeriod = 2 * PI;
const uint32 windTableSize = 64;
Array<float32, windTableSize> windValuesTable;

void Init()
{
    for (int32 i = 0; i < windTableSize; i++) // Taken from wind system.
    {
        float32 t = windPeriod * i / static_cast<float32>(windTableSize);
        windValuesTable[i] = (2.f + std::sin(t) * 0.7f + std::cos(t * 10) * 0.3f) * 0.5f;
    }
    GenerateNoise();
}

void ApplyDragForce(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife);
void ApplyLorentzForce(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife);
void ApplyPointGravity(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife);
void ApplyGravity(const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpaceDown, float32 dt);
void ApplyWind(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife, const Particle* particle);

Vector3 GetForceValue(const ParticleDragForce* force, float32 particleOverLife, float32 layerOverLife);
float32 GetWindValueFromTable(const Vector3& inPosition, const ParticleDragForce* force, float32 layerOverLife, int32 index);

void ApplyForce(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife, const Vector3& effectSpaceDown, const Particle* particle)
{
    using ForceType = ParticleDragForce::eType;

    if (!force->isActive)
        return;
    if (force->type == ForceType::DRAG_FORCE)
    {
        ApplyDragForce(parent, force, effectSpaceVelocity, effectSpacePosition, dt, particleOverLife, layerOverLife);
        return;
    }
    if (force->type == ForceType::LORENTZ_FORCE)
    {
        ApplyLorentzForce(parent, force, effectSpaceVelocity, effectSpacePosition, dt, particleOverLife, layerOverLife);
        return;
    }
    if (force->type == ForceType::GRAVITY)
    {
        ApplyGravity(force, effectSpaceVelocity, effectSpaceDown, dt);
        return;
    }
    if (force->type == ForceType::WIND)
    {
        ApplyWind(parent, force, effectSpaceVelocity, effectSpacePosition, dt, particleOverLife, layerOverLife, particle);
        return;
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
    if (!force->isInfinityRange)
    {
        if (!IsPositionInForceShape(parent, force, effectSpacePosition))
            return;
    }

    Vector3 forceDir = (effectSpacePosition - force->position).CrossProduct(force->direction);
    float32 len = forceDir.SquareLength();
    if (len > 0.0f)
    {
        float32 d = 1.0f / std::sqrt(len);
        forceDir *= d;
    }
    Vector3 forceStrength = GetForceValue(force, particleOverLife, layerOverLife) * dt;
    effectSpaceVelocity += forceStrength * forceDir;
}

void ApplyGravity(const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpaceDown, float32 dt)
{
    effectSpaceVelocity += effectSpaceDown * force->forcePower.z * dt;
}

void ApplyWind(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife, const Particle* particle)
{
    if (!force->isInfinityRange)
    {
        if (!IsPositionInForceShape(parent, force, effectSpacePosition))
            return;
    }
    intptr_t partInd = reinterpret_cast<intptr_t>(particle);
    uint32 particleIndex = *reinterpret_cast<uint32*>(&partInd);
    Vector3 turbulence;
    if (Abs(force->windTurbulence) > EPSILON)
    {
        uint32 offset = particleIndex % noiseWidth;
        uint32 xindex = static_cast<uint32>(floor(particleOverLife * noiseWidth)) + offset;
        float32 fractPart = particleOverLife * noiseWidth + offset - xindex;
        xindex %= noiseWidth;
        uint32 yindex = offset % noiseHeight;
        uint32 nextIndex = (xindex + 1) % noiseWidth;
        Vector3 t1 = noise[xindex][yindex];
        Vector3 t2 = noise[nextIndex][yindex];
        turbulence = Lerp(t1, t2, fractPart);
        float32 dot = Normalize(effectSpaceVelocity).DotProduct(Normalize(turbulence));
        if (dot < 0)
            turbulence *= -1.0f;
        turbulence *= force->windTurbulence * dt;
    }

    effectSpaceVelocity += force->direction * dt * GetWindValueFromTable(effectSpacePosition, force, particleOverLife, particleIndex) + turbulence;
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

float32 GetWindValueFromTable(const Vector3& inPosition, const ParticleDragForce* force, float32 layerOverLife, int32 index)
{
    if (abs(force->windFrequency) < EPSILON)
        return 1.0f;

    Vector3 dir = force->direction;
    Vector3 projPt = dir * inPosition.DotProduct(dir);
    float32 tMod = std::fmod((Abs(index) + layerOverLife) * force->windFrequency, windPeriod);
    int32 i = static_cast<int32>(std::floor(tMod / windPeriod * windTableSize));
    return (std::sin(Abs(index) % 255 + layerOverLife * force->windFrequency) * 0.6f + std::cos(Abs(index) % 255 + layerOverLife * force->windFrequency) * 0.4f) + force->windBias;
    DVASSERT(i >= 0 && i < windTableSize);
    return windValuesTable[i];
}
}
}