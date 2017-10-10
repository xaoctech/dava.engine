#pragma once

#include "Base/BaseObject.h"

#include "Math/Vector.h"
#include "Math/Matrix4.h"
#include "Particles/ParticlePropertyLine.h"

#include "Reflection/Reflection.h"

namespace DAVA
{
struct ParticleLayer;

class ParticleDragForce : public BaseObject
{
public:
    enum class eShape
    {
        BOX,
        SPHERE
    };

    enum class eTimingType
    {
        CONSTANT,
        OVER_LAYER_LIFE,
        OVER_PARTICLE_LIFE,
        SECONDS_PARTICLE_LIFE
    } timingType = eTimingType::CONSTANT;

    enum class eType
    {
        DRAG_FORCE = 0, // Also force priority.
        WIND,
        LORENTZ_FORCE,
        GRAVITY,
        POINT_GRAVITY,
        PLANE_COLLISION
    } type = eType::DRAG_FORCE;

    RefPtr<PropertyLine<Vector3>> forcePowerLine;
    RefPtr<PropertyLine<float32>> turbulenceLine;
    Matrix4 localMatrix;

    Vector3 position;
    Vector3 rotation;
    Vector3 direction{ 0.0f, 0.0f, 1.0f };
    Vector3 forcePower{ 1.0f, 1.0f, 1.0f };

    float32 windFrequency = 0.0f;
    float32 windTurbulenceFrequency = 0.0f;
    float32 windBias = 1.0f;

    float32 windTurbulence = 0.0f;
    float32 pointGravityRadius = 1.0f;
    float32 planeScale = 5.0f; // Editor only.
    float32 reflectionChaos = 0.0f;
    float32 rndReflectionForceMin = 1.0f;
    float32 rndReflectionForceMax = 1.1f;
    float32 velocityThreshold = 0.3f;
    float32 startTime = 0.0f;
    float32 endTime = 15.0f;

    uint32 backwardTurbulenceProbability = 0;
    uint32 reflectionPercent = 100;

    String forceName = "Particle Force";
    bool isActive = true;
    bool isInfinityRange = true;
    bool pointGravityUseRandomPointsOnSphere = false;
    bool isGlobal = false;
    bool killParticles = false;
    bool normalAsReflectionVector = true;
    bool randomizeReflectionForce = false;

    ParticleDragForce(ParticleLayer* parent);
    ParticleDragForce* Clone();
    void GetModifableLines(List<ModifiablePropertyLineBase*>& modifiables);
    void SetRadius(float32 radius);
    void SetBoxSize(const Vector3& boxSize);
    float32 GetRadius() const;
    const Vector3& GetBoxSize() const;
    const Vector3& GetHalfBoxSize() const;
    float32 GetSquareRadius() const;
    void SetShape(eShape shape);
    eShape GetShape() const;

    bool IsForceCanAlterPosition() const;

public:
    DAVA_VIRTUAL_REFLECTION(ParticleDragForce, BaseObject);

private:
    Vector3 boxSize{ 1.0f, 1.0f, 1.0f };
    Vector3 halfBoxSize{ 0.5f, 0.5f, 0.5f };
    float32 squareRadius = 0.75f; // For default box with 0.5f edges.
    float32 radius = 1.0f;
    eShape shape = eShape::BOX;
    ParticleLayer* parentLayer = nullptr;
};

inline void ParticleDragForce::SetRadius(float32 radius_)
{
    radius = radius_;
    if (shape == eShape::SPHERE)
        squareRadius = radius * radius;
}

inline void ParticleDragForce::SetBoxSize(const Vector3& boxSize_)
{
    boxSize = boxSize_;
    halfBoxSize = boxSize * 0.5f;
    if (shape == eShape::BOX)
        squareRadius = halfBoxSize.DotProduct(halfBoxSize);
}

inline float32 ParticleDragForce::GetRadius() const
{
    return radius;
}

inline const Vector3& ParticleDragForce::GetBoxSize() const
{
    return boxSize;
}

inline const Vector3& ParticleDragForce::GetHalfBoxSize() const
{
    return halfBoxSize;
}

inline float32 ParticleDragForce::GetSquareRadius() const
{
    return squareRadius;
}

inline void ParticleDragForce::SetShape(ParticleDragForce::eShape shape_)
{
    shape = shape_;
    if (shape == eShape::BOX)
    {
        halfBoxSize = boxSize * 0.5f;
        squareRadius = boxSize.DotProduct(halfBoxSize);
    }
    else if (shape == eShape::SPHERE)
        squareRadius = radius * radius;
}

inline ParticleDragForce::eShape ParticleDragForce::GetShape() const
{
    return shape;
}

inline bool ParticleDragForce::IsForceCanAlterPosition() const
{
    return type == ParticleDragForce::eType::POINT_GRAVITY || type == ParticleDragForce::eType::PLANE_COLLISION || type == ParticleDragForce::eType::WIND;
}
}
