#include "Particles/ParticleForces.h"

#include <random>
#include <chrono>

#include "Particles/Particle.h"
#include "Particles/ParticleForce.h"
#include "Math/MathHelpers.h"
#include "Math/Noise.h"
#include "Scene3D/Entity.h"

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
        float32 sinPhi = sinf(phi);
        sphereRandomVectors[i] = { sinPhi * cosf(theta), sinPhi * sinf(theta), cosf(phi) };
    }
}

Vector3 GetNoiseValue(float32 particleOverLife, float32 frequency, uint32 clampedIndex)
{
    float32 indexUnclamped = particleOverLife * noiseWidth * frequency + clampedIndex;
    float32 intPart = 0.0f;
    float32 fractPart = modff(particleOverLife * noiseWidth * frequency + clampedIndex, &intPart);
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

inline void KillParticlePlaneCollision(const ParticleForce* force, Particle* particle, Vector3& effectSpaceVelocity)
{
    if (force->killParticles)
        KillParticle(particle);
    else
        effectSpaceVelocity = Vector3::Zero;
}
}

void Init()
{
    ParticleForcesDetail::GenerateNoise();
    ParticleForcesDetail::GenerateSphereRandomVectors();
}

template <typename T>
T GetValue(const ParticleForce* force, float32 particleOverLife, float32 layerOverLife, float32 particleLife, PropertyLine<T>* line, T value)
{
    if (force->timingType == ParticleForce::eTimingType::CONSTANT || line == nullptr)
        return value;

    if (force->timingType == ParticleForce::eTimingType::OVER_PARTICLE_LIFE)
        return line->GetValue(particleOverLife);

    if (force->timingType == ParticleForce::eTimingType::OVER_LAYER_LIFE)
        return line->GetValue(layerOverLife);

    if (force->timingType == ParticleForce::eTimingType::SECONDS_PARTICLE_LIFE)
        return line->GetValue(particleLife);

    return value;
}

void ApplyDragForce(const ParticleForce* force, Vector3& velocity, const Vector3& position, float32 dt, float32 particleOverLife, float32 layerOverLife, const Particle* particle, const Vector3& forcePosition);
void ApplyLorentzForce(const ParticleForce* force, Vector3& velocity, const Vector3& position, float32 dt, float32 particleOverLife, float32 layerOverLife, const Particle* particle, const Vector3& forcePosition);
void ApplyPointGravity(const ParticleForce* force, Vector3& velocity, Vector3& position, float32 dt, float32 particleOverLife, float32 layerOverLife, Particle* particle, const Vector3& forcePosition);
void ApplyGravity(const ParticleForce* force, Vector3& velocity, const Vector3& down, float32 dt, float32 particleOverLife, float32 layerOverLife, const Particle* particle);
void ApplyWind(const ParticleForce* force, Vector3& velocity, Vector3& position, float32 dt, float32 particleOverLife, float32 layerOverLife, const Particle* particle, const Vector3& forcePosition);
void ApplyPlaneCollision(const ParticleForce* force, Vector3& velocity, Vector3& position, Particle* particle, const Vector3& prevPosition, const Vector3& forcePosition);

void ApplyForce(const ParticleForce* force, Vector3& velocity, Vector3& position, float32 dt, float32 particleOverLife, float32 layerOverLife, const Vector3& down, Particle* particle, const Vector3& prevPosition, const Vector3& forcePosition)
{
    using ForceType = ParticleForce::eType;

    if (!force->isActive)
        return;
    if (force->type == ForceType::DRAG_FORCE)
    {
        ApplyDragForce(force, velocity, position, dt, particleOverLife, layerOverLife, particle, forcePosition);
        return;
    }
    if (force->type == ForceType::LORENTZ_FORCE)
    {
        ApplyLorentzForce(force, velocity, position, dt, particleOverLife, layerOverLife, particle, forcePosition);
        return;
    }
    if (force->type == ForceType::GRAVITY)
    {
        ApplyGravity(force, velocity, down, dt, particleOverLife, layerOverLife, particle);
        return;
    }
    if (force->type == ForceType::WIND)
    {
        ApplyWind(force, velocity, position, dt, particleOverLife, layerOverLife, particle, forcePosition);
        return;
    }
    if (force->type == ForceType::POINT_GRAVITY)
    {
        ApplyPointGravity(force, velocity, position, dt, particleOverLife, layerOverLife, particle, forcePosition);
        return;
    }
    if (force->type == ForceType::PLANE_COLLISION)
    {
        ApplyPlaneCollision(force, velocity, position, particle, prevPosition, forcePosition);
        return;
    }
}

inline bool IsPositionInForceShape(const ParticleForce* force, const Vector3& particlePosition, const Vector3& forcePosition)
{
    if (force->isInfinityRange)
        return true;

    if (force->GetShape() == ParticleForce::eShape::BOX)
    {
        AABBox3 box(forcePosition - force->GetHalfBoxSize(), forcePosition + force->GetHalfBoxSize());
        if (box.IsInside(particlePosition))
            return true;
    }
    else if (force->GetShape() == ParticleForce::eShape::SPHERE)
    {
        if ((forcePosition - particlePosition).SquareLength() <= force->GetSquareRadius())
            return true;
    }
    return false;
}

void ApplyDragForce(const ParticleForce* force, Vector3& velocity, const Vector3& position, float32 dt, float32 particleOverLife, float32 layerOverLife, const Particle* particle, const Vector3& forcePosition)
{
    if (!IsPositionInForceShape(force, position, forcePosition))
        return;

    Vector3 forceStrength = GetValue(force, particleOverLife, layerOverLife, particle->life, force->forcePowerLine.Get(), force->forcePower) * dt;
    Vector3 v(Max(0.0f, 1.0f - forceStrength.x), Max(0.0f, 1.0f - forceStrength.y), Max(0.0f, 1.0f - forceStrength.z));
    velocity *= v;
}

void ApplyLorentzForce(const ParticleForce* force, Vector3& velocity, const Vector3& position, float32 dt, float32 particleOverLife, float32 layerOverLife, const Particle* particle, const Vector3& forcePosition)
{
    if (!IsPositionInForceShape(force, position, forcePosition))
        return;

    Vector3 forceDir = (position - forcePosition).CrossProduct(force->direction);
    float32 len = forceDir.SquareLength();
    if (len > 0.0f)
    {
        float32 d = 1.0f / std::sqrt(len);
        forceDir *= d;
    }
    Vector3 forceStrength = GetValue(force, particleOverLife, layerOverLife, particle->life, force->forcePowerLine.Get(), force->forcePower) * dt;
    velocity += forceStrength * forceDir;
}

void ApplyGravity(const ParticleForce* force, Vector3& velocity, const Vector3& down, float32 dt, float32 particleOverLife, float32 layerOverLife, const Particle* particle)
{
    velocity += down * GetValue(force, particleOverLife, layerOverLife, particle->life, force->forcePowerLine.Get(), force->forcePower).x * dt;
}

void ApplyWind(const ParticleForce* force, Vector3& velocity, Vector3& position, float32 dt, float32 particleOverLife, float32 layerOverLife, const Particle* particle, const Vector3& forcePosition)
{
    static const float32 windScale = 100.0f; // Artiom request.

    if (!IsPositionInForceShape(force, position, forcePosition))
        return;

    uintptr_t partInd = reinterpret_cast<uintptr_t>(particle);
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
        position += turbulence;
    }
    if (Abs(force->windFrequency) > EPSILON)
    {
        float32 noiseVal = ParticleForcesDetail::GetNoiseValue(particleOverLife, force->windFrequency, clampedIndex).x;
        windMultiplier = noiseVal + force->windBias;
    }
    Vector3 forceStrength = GetValue(force, particleOverLife, layerOverLife, particle->life, force->forcePowerLine.Get(), force->forcePower) * dt;
    velocity += force->direction * dt * windMultiplier * forceStrength.x * windScale;
}

void ApplyPointGravity(const ParticleForce* force, Vector3& velocity, Vector3& position, float32 dt, float32 particleOverLife, float32 layerOverLife, Particle* particle, const Vector3& forcePosition)
{
    if (!IsPositionInForceShape(force, position, forcePosition))
        return;

    Vector3 toCenter = forcePosition - position;
    float32 sqrToCenterDist = toCenter.SquareLength();
    if (sqrToCenterDist > 0)
        toCenter /= sqrt(sqrToCenterDist);

    Vector3 forceDirection = toCenter;
    if (force->pointGravityUseRandomPointsOnSphere)
    {
        intptr_t partInd = reinterpret_cast<intptr_t>(particle);
        uint32 particleIndex = *reinterpret_cast<uint32*>(&partInd);
        particleIndex %= ParticleForcesDetail::sphereRandomVectorsSize;
        Vector3 forcePositionModified = forcePosition + ParticleForcesDetail::sphereRandomVectors[particleIndex] * force->pointGravityRadius;
        forceDirection = forcePositionModified - position;
        float32 sqrDistToTarget = forceDirection.SquareLength();
        if (sqrDistToTarget > 0)
            forceDirection /= sqrt(sqrDistToTarget);
    }

    Vector3 forceStrength = GetValue(force, particleOverLife, layerOverLife, particle->life, force->forcePowerLine.Get(), force->forcePower) * dt;
    if (sqrToCenterDist > force->pointGravityRadius * force->pointGravityRadius)
        velocity += forceDirection * forceStrength;
    else
    {
        if (force->killParticles)
            ParticleForcesDetail::KillParticle(particle);
        else
            position = forcePosition - force->pointGravityRadius * toCenter;
    }
}

void ApplyPlaneCollision(const ParticleForce* force, Vector3& velocity, Vector3& position, Particle* particle, const Vector3& prevPosition, const Vector3& forcePosition)
{
    if (!IsPositionInForceShape(force, position, forcePosition))
        return;

    Vector3 normal = Normalize(force->direction);
    Vector3 a = prevPosition - forcePosition;
    Vector3 b = position - forcePosition;
    float32 bProj = b.DotProduct(normal);
    float32 aProj = a.DotProduct(normal);
    if (bProj <= 0 && aProj > 0)
    {
        if (velocity.SquareLength() < force->velocityThreshold * force->velocityThreshold)
        {
            ParticleForcesDetail::KillParticlePlaneCollision(force, particle, velocity);
            return;
        }

        Vector3 dir = prevPosition - position;
        float32 abProj = Abs(dir.DotProduct(normal));
        if (abProj < EPSILON)
            return;
        position = position + dir * (-bProj) / abProj;

        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int32> uniInt(0, 99);
        bool reflectParticle = static_cast<uint32>(uniInt(rng)) < force->reflectionPercent;
        if (reflectParticle)
        {
            Vector3 newVel;
            if (force->normalAsReflectionVector)
                newVel = (normal * velocity.Length()); // Artiom request.
            else
                newVel = Reflect(velocity, normal);

            if (Abs(force->reflectionChaos) > EPSILON)
            {
                std::uniform_real_distribution<float32> uni(-DegToRad(force->reflectionChaos), DegToRad(force->reflectionChaos));
                Quaternion q = Quaternion::MakeRotationFastX(uni(rng)) * Quaternion::MakeRotationFastY(uni(rng)) * Quaternion::MakeRotationFastZ(uni(rng));
                newVel = q.ApplyToVectorFast(newVel);
                if (newVel.DotProduct(normal) < 0)
                    newVel = -newVel;
            }
            velocity = newVel * force->forcePower;
            if (force->randomizeReflectionForce)
                velocity *= std::uniform_real_distribution<float32>(force->rndReflectionForceMin, force->rndReflectionForceMax)(rng);
        }
        else
            ParticleForcesDetail::KillParticlePlaneCollision(force, particle, velocity);
    }
    else if (bProj < 0.0f && aProj < 0.0f)
        ParticleForcesDetail::KillParticlePlaneCollision(force, particle, velocity);
}
}
}
