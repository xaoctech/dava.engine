#pragma once

#include "Base/BaseObject.h"
#include "Particles/ParticlePropertyLine.h"

namespace DAVA
{
// Particle Force class is needed to store Particle Force data.
class ParticleForce : public BaseObject
{
public:
    ParticleForce() = default;
    ParticleForce(RefPtr<PropertyLine<Vector3>> force, RefPtr<PropertyLine<float32>> forceOverLife);

    ParticleForce* Clone();
    void GetModifableLines(List<ModifiablePropertyLineBase*>& modifiables);

    RefPtr<PropertyLine<Vector3>> force;
    RefPtr<PropertyLine<float32>> forceOverLife;

public:
    INTROSPECTION_EXTEND(ParticleForce, BaseObject, nullptr)
};
};
