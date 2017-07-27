#pragma once

#include "Base/BaseObject.h"

namespace DAVA
{
class ParticleDrag : public BaseObject
{
public:
    ParticleDrag() = default;

    ParticleDrag* Clone();
public:
    INTROSPECTION_EXTEND(ParticleDrag, BaseObject, nullptr)
};
}
