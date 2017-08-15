#pragma once

#include "Base/BaseObject.h"

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
    } shape = eShape::BOX;
    enum class eType
    {
        DRAG_FORCE
    } type = eType::DRAG_FORCE;

    ParticleDragForce(ParticleLayer* parent);

    ParticleDragForce* Clone();

    Vector3 position;
    Vector3 rotation;

    Vector3 forcePower;

    Matrix4 localMatrix;
    Vector3 boxSize;
    float32 radius = 1.0f;

    bool infinityRange = true;

public:
    INTROSPECTION_EXTEND(ParticleDragForce, BaseObject, nullptr)

private:
    ParticleLayer* parentLayer = nullptr;
};
}
