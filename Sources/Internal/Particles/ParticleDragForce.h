#pragma once

#include "Base/BaseObject.h"

namespace DAVA
{
class ParticleDragForce : public BaseObject
{
public:
    ParticleDragForce() = default;

    ParticleDragForce* Clone();
public:
    INTROSPECTION_EXTEND(ParticleDragForce, BaseObject, nullptr)
};
}
