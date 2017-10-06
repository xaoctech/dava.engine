#include "Particles/ParticleForces.h"

#include "Particles/ParticleDragForce.h"
#include "Scene3D/Entity.h"
#include "Math/MathHelpers.h"
#include "Math/Noise.h"
#include "Particles/Particle.h"

#include <random>
#include <chrono>

namespace DAVA
{
namespace ParticleForces
{
namespace ParticleForcesDetail
{
const int32 noiseWidth = 256;
const int32 noiseHeight = 256;
Array<Array<Vector3, noiseWidth>, noiseHeight> noise;

void GenerateNoise()
{
    float32 xFactor = 2.0f / (noiseWidth - 1.0f);
    float32 yFactor = 2.0f / (noiseHeight - 1.0f);
    for (int32 i = 0; i < noiseWidth; ++i)
    {
        Vector2 q(i * xFactor, 0);
        for (int32 j = 0; j < noiseHeight; ++j)
        {
            q.y = j * yFactor;
            noise[i][j] = Generate2OctavesPerlin(q);
        }
    }
}

const uint32 sphereRandomVectorsSize = 1024;
Array<Vector3, sphereRandomVectorsSize> sphereRandomVectors;
void GenerateSphereRandomVectors()
{
    uint32 seed = static_cast<uint32>(std::chrono::system_clock::now().time_since_epoch().count());
    std::mt19937 generator(seed);
    std::uniform_real_distribution<float32> uinform01(0.0f, 1.0f);
    for (uint32 i = 0; i < sphereRandomVectorsSize; ++i)
    {
        float32 theta = 2 * PI * uinform01(generator);
        float32 phi = acos(1 - 2 * uinform01(generator));
        float32 sinPhi = sin(phi);
        sphereRandomVectors[i] = { sinPhi * cos(theta), sinPhi * sin(theta), cos(phi) };
    }
}

Vector3 GetNoiseValue(float32 particleOverLife, float32 frequency, uint32 clampedIndex)
{
    float32 indexUnclamped = particleOverLife * noiseWidth * frequency + clampedIndex;
    float32 intPart = 0.0f;
    float32 fractPart = modf(particleOverLife * noiseWidth * frequency + clampedIndex, &intPart);
    uint32 xindex = static_cast<uint32>(intPart);

    xindex %= noiseWidth;
    uint32 yindex = clampedIndex % noiseHeight;
    uint32 nextIndex = (xindex + 1) % noiseWidth;
    Vector3 t1 = noise[xindex][yindex];
    Vector3 t2 = noise[nextIndex][yindex];
    return Lerp(t1, t2, fractPart);
}

inline void KillParticle(Particle* particle)
{
    particle->life = particle->lifeTime + 0.1f;
}
}

void Init()
{
    ParticleForcesDetail::GenerateNoise();
    ParticleForcesDetail::GenerateSphereRandomVectors();
}

template <typename T>
T GetValue(const ParticleDragForce* force, float32 particleOverLife, float32 layerOverLife, float32 particleLife, PropertyLine<T>* line, T value)
{
    if (force->timingType == ParticleDragForce::eTimingType::CONSTANT || line == nullptr)
        return value;

    if (force->timingType == ParticleDragForce::eTimingType::OVER_PARTICLE_LIFE)
        return line->GetValue(particleOverLife);

    if (force->timingType == ParticleDragForce::eTimingType::OVER_LAYER_LIFE)
        return line->GetValue(layerOverLife);

    if (force->timingType == ParticleDragForce::eTimingType::SECONDS_PARTICLE_LIFE)
        return line->GetValue(particleLife);

    return value;
}

void ApplyDragForce(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife, const Particle* particle);
void ApplyLorentzForce(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife, const Particle* particle);
void ApplyPointGravity(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife, Particle* particle);
void ApplyGravity(const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpaceDown, float32 dt, float32 particleOverLife, float32 layerOverLife, const Particle* particle);
void ApplyWind(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife, const Particle* particle);
void ApplyPlaneCollision(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife, Particle* particle, const Vector3& prevEffectSpacePosition);

void ApplyForce(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife, const Vector3& effectSpaceDown, Particle* particle, const Vector3& prevEffectSpacePosition)
{
    using ForceType = ParticleDragForce::eType;

    if (!force->isActive)
        return;
    if (force->type == ForceType::DRAG_FORCE)
    {
        ApplyDragForce(parent, force, effectSpaceVelocity, effectSpacePosition, dt, particleOverLife, layerOverLife, particle);
        return;
    }
    if (force->type == ForceType::LORENTZ_FORCE)
    {
        ApplyLorentzForce(parent, force, effectSpaceVelocity, effectSpacePosition, dt, particleOverLife, layerOverLife, particle);
        return;
    }
    if (force->type == ForceType::GRAVITY)
    {
        ApplyGravity(force, effectSpaceVelocity, effectSpaceDown, dt, particleOverLife, layerOverLife, particle);
        return;
    }
    if (force->type == ForceType::WIND)
    {
        ApplyWind(parent, force, effectSpaceVelocity, effectSpacePosition, dt, particleOverLife, layerOverLife, particle);
        return;
    }
    if (force->type == ForceType::POINT_GRAVITY)
    {
        ApplyPointGravity(parent, force, effectSpaceVelocity, effectSpacePosition, dt, particleOverLife, layerOverLife, particle);
        return;
    }
    if (force->type == ForceType::PLANE_COLLISION)
    {
        ApplyPlaneCollision(parent, force, effectSpaceVelocity, effectSpacePosition, dt, particleOverLife, layerOverLife, particle, prevEffectSpacePosition);
        return;
    }
}

inline bool IsPositionInForceShape(const Entity* parent, const ParticleDragForce* force, const Vector3& effectSpacePosition)
{
    if (force->isInfinityRange)
        return true;

    if (force->GetShape() == ParticleDragForce::eShape::BOX)
    {
        AABBox3 box(force->position - force->GetHalfBoxSize(), force->position + force->GetHalfBoxSize());
        if (box.IsInside(effectSpacePosition))
            return true;
    }
    else if (force->GetShape() == ParticleDragForce::eShape::SPHERE)
    {
        if ((force->position - effectSpacePosition).SquareLength() <= force->GetSquareRadius())
            return true;
    }
    return false;
}

void ApplyDragForce(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife, const Particle* particle)
{
    if (!IsPositionInForceShape(parent, force, effectSpacePosition))
        return;

    Vector3 forceStrength = GetValue(force, particleOverLife, layerOverLife, particle->life, force->forcePowerLine.Get(), force->forcePower) * dt;
    Vector3 v(Max(0.0f, 1.0f - forceStrength.x), Max(0.0f, 1.0f - forceStrength.y), Max(0.0f, 1.0f - forceStrength.z));
    effectSpaceVelocity *= v;
}

void ApplyLorentzForce(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife, const Particle* particle)
{
    if (!IsPositionInForceShape(parent, force, effectSpacePosition))
        return;

    Vector3 forceDir = (effectSpacePosition - force->position).CrossProduct(force->direction);
    float32 len = forceDir.SquareLength();
    if (len > 0.0f)
    {
        float32 d = 1.0f / std::sqrt(len);
        forceDir *= d;
    }
    Vector3 forceStrength = GetValue(force, particleOverLife, layerOverLife, particle->life, force->forcePowerLine.Get(), force->forcePower) * dt;
    effectSpaceVelocity += forceStrength * forceDir;
}

void ApplyGravity(const ParticleDragForce* force, Vector3& effectSpaceVelocity, const Vector3& effectSpaceDown, float32 dt, float32 particleOverLife, float32 layerOverLife, const Particle* particle)
{
    effectSpaceVelocity += effectSpaceDown * GetValue(force, particleOverLife, layerOverLife, particle->life, force->forcePowerLine.Get(), force->forcePower).x * dt;
}

void ApplyWind(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife, const Particle* particle)
{
    static const float32 windScale = 100.0f; // Artiom request.

    if (!IsPositionInForceShape(parent, force, effectSpacePosition))
        return;

    intptr_t partInd = reinterpret_cast<intptr_t>(particle);
    uint32 particleIndex = *reinterpret_cast<uint32*>(&partInd);
    Vector3 turbulence;

    uint32 clampedIndex = particleIndex % ParticleForcesDetail::noiseWidth;
    float32 windMultiplier = 1.0f;
    float32 tubulencePower = GetValue(force, particleOverLife, layerOverLife, particle->life, force->turbulenceLine.Get(), force->windTurbulence);
    if (Abs(tubulencePower) > EPSILON)
    {
        turbulence = ParticleForcesDetail::GetNoiseValue(particleOverLife, force->windTurbulence, clampedIndex);
        if ((100 - force->backwardTurbulenceProbability) > clampedIndex % 100)
        {
            float32 dot = Normalize(force->direction).DotProduct(Normalize(turbulence));
            if (dot < 0.0f)
                turbulence *= -1.0f;
        }
        turbulence *= tubulencePower * dt;
        effectSpacePosition += turbulence;
    }
    if (Abs(force->windFrequency) > EPSILON)
    {
        float32 noiseVal = ParticleForcesDetail::GetNoiseValue(particleOverLife, force->windFrequency, clampedIndex).x;
        windMultiplier = noiseVal + force->windBias;
    }
    Vector3 forceStrength = GetValue(force, particleOverLife, layerOverLife, particle->life, force->forcePowerLine.Get(), force->forcePower) * dt;
    effectSpaceVelocity += force->direction * dt * windMultiplier * forceStrength.x * windScale;
}

void ApplyPointGravity(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife, Particle* particle)
{
    if (!force->isInfinityRange)
    {
        if (!IsPositionInForceShape(parent, force, effectSpacePosition))
            return;
    }
    Vector3 toCenter = force->position - effectSpacePosition;
    float32 sqrToCenterDist = toCenter.SquareLength();
    if (sqrToCenterDist > 0)
        toCenter /= sqrt(sqrToCenterDist);

    Vector3 forceDirection = toCenter;
    if (force->pointGravityUseRandomPointsOnSphere)
    {
        intptr_t partInd = reinterpret_cast<intptr_t>(particle);
        uint32 particleIndex = *reinterpret_cast<uint32*>(&partInd);
        particleIndex %= ParticleForcesDetail::sphereRandomVectorsSize;
        Vector3 forcePosition = force->position + ParticleForcesDetail::sphereRandomVectors[particleIndex] * force->pointGravityRadius;
        forceDirection = forcePosition - effectSpacePosition;
        float32 sqrDistToTarget = forceDirection.SquareLength();
        if (sqrDistToTarget > 0)
            forceDirection /= sqrt(sqrDistToTarget);
    }

    Vector3 forceStrength = GetValue(force, particleOverLife, layerOverLife, particle->life, force->forcePowerLine.Get(), force->forcePower) * dt;
    if (sqrToCenterDist > force->pointGravityRadius * force->pointGravityRadius)
        effectSpaceVelocity += forceDirection * forceStrength;
    else
    {
        if (force->killParticles)
            ParticleForcesDetail::KillParticle(particle);
        else
            effectSpacePosition = force->position - force->pointGravityRadius * toCenter;
    }
}

void ApplyPlaneCollision(Entity* parent, const ParticleDragForce* force, Vector3& effectSpaceVelocity, Vector3& effectSpacePosition, float32 dt, float32 particleOverLife, float32 layerOverLife, Particle* particle, const Vector3& prevEffectSpacePosition)
{
    float32 sqrLen = force->direction.SquareLength();
    if (sqrLen < EPSILON * EPSILON)
        return;
    if (!force->isInfinityRange)
    {
        if (!IsPositionInForceShape(parent, force, effectSpacePosition))
            return;
    }
    Vector3 normal = force->direction;
    float32 invLen = 1.0f / sqrt(sqrLen);
    normal *= invLen;
    Vector3 a = prevEffectSpacePosition - force->position;
    Vector3 b = effectSpacePosition - force->position;
    float32 bProj = b.DotProduct(normal);
    if (bProj <= 0 && a.DotProduct(normal) > 0)
    {
        if (effectSpaceVelocity.SquareLength() < force->velocityThreshold * force->velocityThreshold)
        {
            effectSpaceVelocity = Vector3::Zero;
            return;
        }
        std::random_device rd;
        std::mt19937 rng(rd());
        std::uniform_int_distribution<int32> uniInt(0, 99);
        int32 rnd100 = uniInt(rng);
        bool reflectParticle = static_cast<uint32>(rnd100) < force->reflectionPercent;
        if (force->killParticles && !reflectParticle)
        {
            ParticleForcesDetail::KillParticle(particle);
            return;
        }

        Vector3 newVel;
        if (force->normalAsReflectionVector)
            newVel = (normal * effectSpaceVelocity.Length()); // Artiom request.
        else
            newVel = Reflect(effectSpaceVelocity, normal);

        Vector3 rndVec;
        Quaternion q;

        if (abs(force->reflectionChaos) > EPSILON)
        {
            intptr_t partInd = reinterpret_cast<intptr_t>(particle);
            uint32 particleIndex = *reinterpret_cast<uint32*>(&partInd);
            particleIndex %= ParticleForcesDetail::sphereRandomVectorsSize;
            rndVec = ParticleForcesDetail::sphereRandomVectors[particleIndex];
            if (rndVec.DotProduct(normal) < 0)
                rndVec = -rndVec;

            std::uniform_real_distribution<float32> uni(-force->reflectionChaos, force->reflectionChaos);

            float32 random_x = DegToRad(uni(rng));
            float32 random_y = DegToRad(uni(rng));
            float32 random_z = DegToRad(uni(rng));
            q = Quaternion::MakeRotationFastX(random_x) * Quaternion::MakeRotationFastY(random_y) * Quaternion::MakeRotationFastZ(random_z);
            newVel = q.ApplyToVectorFast(newVel);
            if (newVel.DotProduct(normal) < 0)
                newVel = -newVel;
        }
        effectSpaceVelocity = newVel * force->forcePower;
        if (force->randomizeReflectionForce)
        {
            std::uniform_real_distribution<float32> uni(force->rndReflectionForceMin, force->rndReflectionForceMax);
            effectSpaceVelocity *= uni(rng);
        }

        Vector3 dir = prevEffectSpacePosition - effectSpacePosition;
        float32 abProj = abs(dir.DotProduct(normal));
        if (abProj < EPSILON)
            return;

        effectSpacePosition = effectSpacePosition + dir * (-bProj) / abProj;

        if (!reflectParticle)
        {
            if (force->killParticles)
                particle->life = particle->lifeTime + 0.1f;
            else
                effectSpaceVelocity = Vector3::Zero;
        }
    }
    else if ((bProj < 0.0f && a.DotProduct(normal) < 0.0f))
    {
        if (force->killParticles)
            particle->life = particle->lifeTime + 0.1f;
        else
            effectSpaceVelocity = Vector3::Zero;
    }
}
}
}